//world.c
/******************************************************************************
* WORLD SIMULATION
*******************************************************************************
* Advances the game world (stored in  game_state ) by one step and provides
* methods for manipulating the world, which don't relate specifically to the
* player or the enemies, like  add_laser() ,  detect_collision() , etc.
******************************************************************************/

#include "world.h"
#include "main.h"
#include "game.h"
#include "level_design.h"
#include "enemy.h"
#include "player.h"
#include "ui.h"

/* remove_all_objects()
 * Initializes the arrays holding the currently active (visible) objects in the
 * game world, like lasers, enemies, etc. Used by  reset_game() .

 * See also
 * 	world.c
 * 	add_laser_beam
 * 	add_explosion
 * 	add_bonus_bubble
 *
 * 	level_design.c
 * 	add_random_enemy
 *
 * and their corresponding remove functions.
 */
void remove_all_objects( game_state_t* GS )
{
	int i;

	GS->nr_active_enemies_total = 0;
	for( i = 0 ; i < NR_TIERS ; i++ ) {
		GS->nr_active_enemies[i] = 0;
	}
	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {
		GS->enemies[i].active = FALSE;
		GS->enemies[i].formation = NULL;
	}

	GS->nr_active_lasers = 0;
	for( i = 0 ; i < MAX_LASER_BEAMS ; i++ ) {
		GS->laser_beams[i].active = FALSE;
	}

	GS->nr_active_explosions = 0;
	for( i = 0 ; i < MAX_EXPLOSIONS ; i++ ) {
		GS->explosions[i].active = FALSE;
	}

	GS->nr_active_bonus_bubbles_total = 0;
	for( i = 0 ; i < NR_TIERS ; i++ ) {
		GS->nr_active_bonus_bubbles[i] = 0;
	}
	for( i = 0 ; i < MAX_BONUS_BUBBLES ; i++ ) {
		GS->bonus_bubbles[i].active = FALSE;
	}
}

/* disable_weapons()
 * Sets all player's weapons to disabled. Used by  reset_game() .
 */
void disable_weapons( game_state_t* GS )
{
	int i;

	for( i = 0 ; i < NR_WEAPONS ; i++ ) {
		GS->ship.weapons[i].blink_until_us = 0;
		GS->ship.weapons[i].enabled
#if CHEAT_MODE
			= TRUE;
#else
			= FALSE;
#endif
	}

}

/* add_laser_beam()
 * Creates a new entry in  laser_beams[] , setting a new beam en route.
 */
void add_laser_beam(
	game_state_t* GS,	// All game related data
	int new_owner,		// Who fired this beam? If < 0, it was the
				// player, the number indicates the mode
				// (Single, dual, round shot).
	vector_t new_position,	// From where
	vector_t new_velocity,	// In which direction
	real_t new_speed_bonus	// At which speed did the player fire?
				// (A kill will result in less Resource gain,
				// when fired at slow speed)
) {
	int i;
	laser_beam_t* l;

	if (new_owner >= 0) {
		//...GS->enemies[new_owner].tier ... Aim!
	}
	else {	// The player initiated this shot
		new_velocity.y += GS->ship.velocity.y;
		++GS->shots_en_route;
	}

	// Search for an empty slot and store the data
	for( i = 0 ; i < MAX_LASER_BEAMS ; i++ ) {

		l = &(GS->laser_beams[i]);

		if (l->active == FALSE) {
			l->position       = new_position;
			l->velocity       = new_velocity;
			l->decay_beyond_y = new_position.y + FIELD_HEIGHT/2;
			l->speed_bonus    = new_speed_bonus;
			l->owner          = new_owner;

			l->active = TRUE;
			GS->nr_active_lasers++;

			if (new_owner < 0) {		// Player?
				GS->shots_fired++;	// Count this shot
			}

			return;
		}
	}

	fprintf( stderr, "Could not add laser_beam: all slots busy.\n" );
}

void remove_laser_beam( game_state_t* GS, laser_beam_t* l )
{
	l->active = FALSE;
	GS->nr_active_lasers--;

	if (l->owner < 0) {	// The player initiated this shot
		--GS->shots_en_route;
	}
}

/* add_explosion()
 * Creates a new entry in the  explosions[]  array.
 */
void add_explosion(
	program_state_t* PS,
	game_state_t* GS,
	vector_t new_position
	)
{
	int i;
	explosion_t* e;

	// Search for an empty slot and store the data
	for( i = 0 ; i < MAX_EXPLOSIONS ; i++ ) {

		e = &(GS->explosions[i]);

		if (e->active == FALSE) {
			e->position      = new_position;
			e->start_time_us = PS->game_time_us;

			e->active = TRUE;
			GS->nr_active_explosions++;

			return;
		}
	}

	fprintf( stderr, "Could not add explosion: all slots busy.\n" );
}

void remove_explosion( game_state_t* GS, explosion_t* e )
{
	e->active = FALSE;
	GS->nr_active_explosions--;
}

/* add_bonus_bubble()
 */
void add_bonus_bubble(
	program_state_t* PS,
	game_state_t* GS,
	vector_t new_position,
	color_t new_color,
	int new_tier,
	int new_resource
	)
{
	int i;
	bonus_bubble_t* b;

	// Search for an empty slot and store the data
	for( i = 0 ; i < MAX_BONUS_BUBBLES ; i++ ) {

		b = &(GS->bonus_bubbles[i]);

		if (b->active == FALSE) {
			b->position = new_position;
			b->color    = new_color;
			b->resource = new_resource;
			b->tier     = new_tier;

			b->start_time_us = PS->game_time_us;	//...check other objects for using current_time_us

			b->active = TRUE;
			GS->nr_active_bonus_bubbles_total++;
			GS->nr_active_bonus_bubbles[ b->tier ]++;

			return;
		}
	}

	fprintf( stderr, "Could not add bonus bubble: all slots busy.\n" );
}

void remove_bonus_bubble( game_state_t* GS, bonus_bubble_t* b )
{
	b->active = FALSE;
	GS->nr_active_bonus_bubbles[ b->tier ]--;
	GS->nr_active_bonus_bubbles_total--;
}


/******************************************************************************
* DETECT COLLISION
******************************************************************************/

/* detect_collision()
 * Checks, if two objects come nearer than a given distance.
 * This function has to be called, AFTER both objects have been moved,
 * therefore  velocity  has to be subtracted from  position .
 */
bool_t detect_collision(
	real_t tick_fraction_s,
	vector_t position1, vector_t velocity1,
	vector_t position2, vector_t velocity2,
	real_t distance
	)
{
	//... Improve me! When frame rate is low (on slow computers), a hit
	//... might not be captured by this algorithm, due to a too large
	//... step size.

	// My old check. Simple, but works at 50 FPS or better:
	real_t d = vector_length( subtract_vector( position1, position2 ) );

	if (d <= distance) {
		return TRUE;
	}

	//...Check me!
	//...Mathias' solution appears to ignore some hits (1 of 100 or so)

	/* Mathias' Solution
	 * x0 = px0 + vx0 * t,	x1 = px1 + vx1 * t
	 * y0 = py0 + vy0 * t,	y1 = py1 + vy1 * t
	 * z0 = pz0 + vz0 * t,	z1 = pz1 + vz1 * t
	 *
	 * (x0-x1)^2 + (y0-y1)^2 + (z0 - z1)^2 = r^2
	 *
	 * At^2 + Bt + C = 0:
	 *  A = Dvx^2 + Dvy^2 + Dvz^2
	 *  B = 2 * (Dpx * Dvx + Dpy * Dvy + Dpz * Dvz)
	 *  C = Dpx^2 + Dpy^2 + Dpz^2 - r^2
	 *
	 *          -B (+/-) sqrt( B^2 - 4AC )
	 * x(1,2) = --------------------------
	 *                     2A
	 *
	 * If there is a solution, we know, that we came at least as near as r.
	 * Solution exists, if (B^2 >= 4AC).
	 */

	real_t Dpx = position2.x - position1.x;
	real_t Dpy = position2.y - position1.y;

	real_t Dvx = velocity2.x - velocity1.x;
	real_t Dvy = velocity2.y - velocity1.y;

	real_t A = Dvx*Dvx + Dvy*Dvy;
	real_t B = 2.0 * (Dpx*Dvx + Dpy*Dvy);
	real_t C = Dpx*Dpx + Dpy*Dpy - distance*distance;

	if ((B < 0) && (B*B >= 4*A*C)) {
		//real_t t0 = -B + sqrt( B*B - 4*A*C );
		//real_t t1 = -B - sqrt( B*B - 4*A*C );
		//real_t t = (t0 + t1) / 2;

		real_t Tmid = -B / (2*A);	// Time of nearest approach

		return (Tmid <= tick_fraction_s);
	}
	else {
		return FALSE;
	}
}


/******************************************************************************
* ADVANCE SIMULATION
******************************************************************************/

// PLAYER /////////////////////////////////////////////////////////////////////

/* advance_ship()
 * Collision detection is done in advance_enemies() and advance_laser_beams()
 * (Handles transition between camera (speed related) angles
 */
void advance_ship( program_state_t* PS, game_state_t* GS )
{
	ship_t* s = &(GS->ship);
	thruster_state_t* ts = &(s->thruster_state);
	vector_t* v = &(s->velocity);

#if LIMIT_FORWARD_SPEEDING
	// Stop speeding after a while
	if (PS->current_time_us > s->thruster_state.forward_until_us) {
		ts->forward = FALSE;
	}
#endif

	v->x = 0.0;
	v->y = SPEED_Y;
	v->z = 0.0;

	// Calculate current velocity vector
	if (ts->right) {
		v->x += SPEED_X;
	}
	if (ts->left) {
		v->x -= SPEED_X;
	}
	if (ts->back) {
		v->y *= BACK_SPEED_FACTOR;
	}

	if (ts->forward) {

#if LIMIT_FORWARD_SPEEDING
		if (PS->current_time_us > ts->pay_after_us) {
			ts->pay_after_us
				= PS->current_time_us
				+ FORWARD_THRUSTING_PAY_TIME
			;
			if (GS->current_resource < FORWARD_RESOURCE_COST) {
				ts->forward = FALSE;
				GS->current_resource = 0;
			}
			else {
				GS->current_resource -= FORWARD_RESOURCE_COST;
			}
		}
#endif
		v->y *= FORWARD_SPEED_FACTOR;
	}

	// Update the position
	s->position.x += v->x * PS->tick_fraction_s;
	s->position.y += v->y * PS->tick_fraction_s;

	// Check for horizontal boundaries and clamp the position
	if (s->position.x < SHIP_MIN_X) {
		s->position.x = SHIP_MIN_X;
	}
	else if (s->position.x > SHIP_MAX_X) {
		s->position.x = SHIP_MAX_X;
	}

	/* Camera Speed-Pitch Adjustment
	 * When speed is changed (Up/Down keys), the camera changes smoothly
	 * to the new position
	 */
	if (v->y > SPEED_Y) {
		s->speed_target_pitch = +1.0;
	}
	else if (v->y < SPEED_Y) {
		s->speed_target_pitch = -1.0;
	}
	else {
		s->speed_target_pitch = 0;
	}

	//...s->speed_target_pitch = -v.y / 10.0;

	if (s->camera_speed_pitch < s->speed_target_pitch) {

		s->camera_speed_pitch
			+= CAMERA_ROTATION_CHANGE_SPEED
			* PS->tick_fraction_s
		;

		if (s->camera_speed_pitch > s->speed_target_pitch) {
			s->camera_speed_pitch
				= s->speed_target_pitch;
		}
	}
	else if (s->camera_speed_pitch > s->speed_target_pitch) {

		s->camera_speed_pitch
			-= CAMERA_ROTATION_CHANGE_SPEED
			* PS->tick_fraction_s
		;

		if (s->camera_speed_pitch < s->speed_target_pitch) {
			s->camera_speed_pitch
				= s->speed_target_pitch;
		}
	}
}


// LASER BEAM /////////////////////////////////////////////////////////////////

/* advance_laser_beam()
 * Gets an active laser beam and calculates its interactions
 */
void advance_laser_beam(
	program_state_t* PS,
	game_state_t* GS,
	laser_beam_t* l
	)
{
	int i;
	ship_t* s = &(GS->ship);
	enemy_t* e;

	l->position.x += l->velocity.x * PS->tick_fraction_s;
	l->position.z += l->velocity.z * PS->tick_fraction_s;

	if (l->owner < 0) {
		l->position.y += l->velocity.y * PS->tick_fraction_s;
	}
	else {	// Enemy
		l->position.y
			+= (0.4 + (real_t)GS->current_level / 10.0)
			* l->velocity.y
			* PS->tick_fraction_s
		;
	}

	// Beam leaving the game area?
	if (	(l->position.y > l->decay_beyond_y)
	||	(l->position.y - s->position.y < -FIELD_HEIGHT/2)//...
	||	(l->position.y - s->position.y > +FIELD_HEIGHT/2)//...
	||	(l->position.x < FIELD_MIN_X)
	||	(l->position.x > FIELD_MAX_X)
	) {
		if (l->owner < 0) {	// Player
			GS->shots_missed++;
		}
		remove_laser_beam( GS, l );
		return;
	}

	// hitting player's ship?
	if(TRUE){//... (l->owner >= 0) {	// No self-kills

		if (detect_collision(
			PS->tick_fraction_s,
			l->position, l->velocity,
			s->position, s->velocity,
			SHIP_SIZE
			)
		) {
			player_takes_hit( PS, GS, l );
			remove_laser_beam( GS, l );
			return;
		}
	}

	// hitting enemies?
	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {

		e = &(GS->enemies[i]);

		if ((e->active) && (l->owner != i)) {	// No self-kills

			if (detect_collision(
				PS->tick_fraction_s,
				l->position, l->velocity,
				e->position, e->velocity,
				enemy_size(GS, e)
				)
			) {
				enemy_takes_hit( PS, GS, e, l );
				remove_laser_beam( GS, l );
				return;
			}
		}
	}
}

/* advance_laser_beams()
 * Calculates all interactions with all currently active beams
 */
void advance_laser_beams( program_state_t* PS, game_state_t* GS )
{
	int i;

	for( i = 0 ; i < MAX_LASER_BEAMS ; i++ ) {
		if (GS->laser_beams[i].active) {
			advance_laser_beam( PS, GS, &(GS->laser_beams[i]) );
		}
	}
}


// BONUS BUBBLES //////////////////////////////////////////////////////////////

void advance_bonus_bubbles( program_state_t* PS, game_state_t* GS )
{
	int i;

	ship_t* shp = &(GS->ship);
	bonus_bubble_t* bb;
	weapon_t* wpn;
	int weapon_nr = -1;

	const vector_t v0 = { 0,0,0 };

	for( i = 0 ; i < MAX_BONUS_BUBBLES ; i++ ) {

		bb = &(GS->bonus_bubbles[i]);

		if (bb->active) {

			//...? decay bubble value

			// Collision with ship
			if (detect_collision(
				PS->tick_fraction_s,
				bb->position, v0,
				shp->position, shp->velocity,
				bubble_size(bb) + SHIP_SIZE
				)
			) {
				remove_bonus_bubble( GS, bb );

				// Free up more weapons
				switch (bb->tier) {
				case TIER_1:
					weapon_nr = WEAPON_AUTOFIRE;
					wpn = &(shp->weapons[weapon_nr]);
					break;
				case TIER_2:
					weapon_nr = WEAPON_LASER_2;
					wpn = &(shp->weapons[weapon_nr]);
					break;
				case TIER_3:
					weapon_nr = WEAPON_ROUNDSHOT;
					wpn = &(shp->weapons[weapon_nr]);
					break;
				default:
					wpn = NULL;
				}

				if (wpn != NULL) {
					if (!wpn->enabled) {
						blink_weapon_hud(
							PS,
							wpn,
						ACHIEVEMENT_BLINK_DURATION
						);
#if PLAY_COMPUTER_VOICE
switch( weapon_nr ) {
	case WEAPON_AUTOFIRE:  play_sound( GS->sounds.computer_autofire );   break;
	case WEAPON_LASER_2:   play_sound( GS->sounds.computer_doubleshot ); break;
	case WEAPON_ROUNDSHOT: play_sound( GS->sounds.computer_roundshot );  break;
}
#endif

					}
					wpn->enabled = TRUE;
				}

				GS->current_resource += bb->resource;

				play_sound( GS->sounds.blub );
			}

			// Remove bubbles that are way behind the player
			if (bb->position.y - shp->position.y
				< -FIELD_HEIGHT / 8)
			{
				bb->position.y
					= shp->position.y
					+ FIELD_HEIGHT / 2
				;
			}
		}
	}
}


// BLACK HOLE /////////////////////////////////////////////////////////////////

void advance_black_hole( program_state_t* PS, game_state_t* GS )
{
	ship_t* s = &(GS->ship);

	real_t Sx = s->position.x;
	real_t Sy = s->position.y;

	// Let the Black Hole move around
	black_hole_t* bh = &(GS->black_hole);
	bh->position = add_vector( bh->position, bh->velocity );
	if ( 	(bh->position.x + bh->velocity.x > FIELD_MAX_X)
	||	(bh->position.x + bh->velocity.x < FIELD_MIN_X)
	) {
		bh->velocity.x *= (-1);
	}

	real_t Bx = GS->black_hole.position.x;
	real_t By = GS->black_hole.position.y;

	s->distance_to_black_hole = sqrt(
		  (Sx - Bx)
		* (Sx - Bx)
		+ (Sy - By - BLACK_HOLE_CHEAT_OFFSET_Y)
		* (Sy - By - BLACK_HOLE_CHEAT_OFFSET_Y)
	);

#if BLACK_HOLES_SUCK
	real_t previous_resource;

	if (s->distance_to_black_hole < BLACK_HOLE_RADIUS_RESOURCE) {

		previous_resource = GS->current_resource;

		GS->current_resource
			= GS->current_resource
			- GS->current_resource
			* PS->tick_fraction_s
			* BLACK_HOLE_RADIUS_RESOURCE
			/ s->distance_to_black_hole
		;

		if ((previous_resource > LASER_HIT_PENALTY_MIN)
		&& (GS->current_resource <= LASER_HIT_PENALTY_MIN)
		) {
			play_sound( GS->sounds.alarm );
		}
	}
#endif

	if (GS->black_hole.position.y - s->position.y < -FIELD_HEIGHT) {
		generate_black_hole( GS );
	}

#if BLACK_HOLES_SUCK
	blackhole_attracts_ship( GS );

	if ((PS->run_mode & RM_RUNNING)
	&& (fabs(GS->black_hole.camera_rotation.z) > 30.0)
	) {
		GS->current_resource = 0.0;
		PS->run_mode = RM_AFTER_LIFE;
		PS->after_life_start_us = get_time();
	}
#endif
}


// TRIGGERED EVENTS ///////////////////////////////////////////////////////////

//... move me to level_design.c
void advance_every_second( program_state_t* PS, game_state_t* GS )
{
	PS->next_second_us = PS->current_time_us + 1000000;

	// Auto-score for traveled distance
	if ((GS->next_recharge_beyond_y < GS->ship.position.y)
	&& (GS->next_recharge_after_us < PS->current_time_us))
	{
		GS->current_resource
			+= RECHARGE_RESOURCE_AMOUNT
#if CHEAT_MODE
			* CHEAT_RESOURCE_FACTOR;
#endif
		;
		GS->next_recharge_beyond_y
			= GS->ship.position.y
			+ RECHARGE_DISTANCE
		;
		GS->next_recharge_after_us
			= PS->current_time_us
			+ RECHARGE_TIME
		;
	}

	if (GS->current_resource < 0) {
		GS->current_resource = 0;
	}

	//... add time based new enemies here
}


// ADVANCE SIMULATION /////////////////////////////////////////////////////////

/* advance_simulation()
 * Calculates all "physics" for the current frame.
 */
void advance_simulation( program_state_t* PS, game_state_t* GS )
{
	ship_t* s = &(GS->ship);

	switch (PS->run_mode) {
	case RM_AFTER_LIFE:
		// Switch to main menu after some time?
		if (PS->current_time_us - PS->after_life_start_us
			> AFTER_LIFE_DURATION_US)
		{
			PS->run_mode = RM_MAIN_MENU;
			PS->main_menu_since_us	= PS->current_time_us;

			// Make the following look nice in debug info
			PS->game_start_us	=
			PS->pause_since_us	= PS->program_start_us - 1;
		}
		break;

	case RM_RUNNING:
		// Events occurring once in a second
		if (PS->next_second_us < PS->current_time_us) {
			advance_every_second( PS, GS );
		}

		// Update  best_resource  (shown in HUD and used for score)
		if (GS->current_resource > GS->best_resource) {
			GS->best_resource = GS->current_resource;
		}

		// Adding enemies, when player "warped" around one field height
		if (s->position.y > GS->add_enemy_beyond_y) {
			player_warped_around( GS );
		}

		simulate_enemy_ai( PS, GS );	// Let them think and decide
						// (Mainly firing, aiming)
		advance_ship( PS, GS );
		advance_enemies( PS, GS );	// Actually move the objects..
		advance_laser_beams( PS, GS );	// ..and collision detection
		advance_bonus_bubbles( PS, GS );
		advance_black_hole( PS, GS );

		if (GS->nr_active_enemies_total == 0) {
			advance_to_next_level( PS, GS );
		}

#if DISABLED_CODE
		if (s->position.y / 100 / 30 > GS->current_level) {
			advance_to_next_level( PS, GS );
			//... missing: keep existing formations
		}
#endif

		break;
	}
}


//EOF
