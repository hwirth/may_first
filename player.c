//player.c
/******************************************************************************
* PLAYER CONTROL
*******************************************************************************
* Player related functions, that could otherwise be put in  game.c .
* E.g. functions that provide methods for the user interface, like reacting
* to a "move forward" command from the joystick, and reactions to world events
* that could be put in  world.c , like taking a hit (and loosing Resource).
******************************************************************************/

#include "player.h"
#include "main.h"
#include "game.h"
#include "world.h"	// add_laser_beam()
#include "ui.h"	// play_sound()


/* start_fire()
 * Carry out the trigger command from the player. Sets some variables in
 * preparation of a possible continuation later on ("auto fire").
 */
void start_fire(
	program_state_t* PS,
	game_state_t* GS,
	int weapon_nr,
	bool_t continuing_auto_fire
) {
	int new_owner;
	weapon_t* wpn = &GS->ship.weapons[weapon_nr];
	vector_t new_position;
	vector_t new_velocity;
	real_t new_speed_bonus;

	int cost = (weapon_nr+1)*(weapon_nr+1);

	if ((wpn->enabled) && (GS->current_resource >= cost)) {

#if FIRE_DIAGONALLY
		new_velocity.x = GS->ship.velocity.x * MOVING_ANGLE_OFFSET / SPEED_X;
#else
		new_velocity.x = 0.0;
#endif
		new_velocity.z = 0.0;
		new_velocity.y = GS->ship.velocity.y;

		new_owner = OWNER_PLAYER;
		new_position = GS->ship.position;
		new_velocity.y = LASER_SPEED_PLAYER;
		new_speed_bonus = (GS->ship.thruster_state.back) ? 0.5 : 1.0 ;

		GS->current_resource -= cost;

		if (continuing_auto_fire) {
			wpn->fire_next_beam_us
				= PS->current_time_us
				+ AUTO_FIRE_INTERVAL
			;
			wpn->auto_fire_count++;
		}
		else {	// First shot of autofire
			wpn->fire_next_beam_us
				= PS->current_time_us
				+ AUTO_FIRE_DELAY
			;
			wpn->auto_fire_count = 1;
		}

		// Add laser beams
		if (weapon_nr == WEAPON_LASER_1) {
			add_laser_beam(
				GS, new_owner, new_position,
				new_velocity, new_speed_bonus
			);
		}
		else if (weapon_nr == WEAPON_LASER_2) {

			new_position.x -= DUAL_FIRE_DISTANCE/2;
			add_laser_beam(
				GS, new_owner, new_position,
				new_velocity, new_speed_bonus
			);

			new_position.x += DUAL_FIRE_DISTANCE;
			add_laser_beam(
				GS, new_owner, new_position,
				new_velocity, new_speed_bonus
			);
		}
		play_sound( GS->sounds.laser );
	}
	else {
		blink_weapon_hud( PS, wpn, DENIED_BLINK_DURATION );
		play_sound( GS->sounds.denied );
	}
}

/* start_round_shot()
 * Calculates directions around the ship and weighs them to have more shots
 * fired in forward direction.
 */
void start_round_shot(
	program_state_t* PS,
	game_state_t* GS,
	bool_t continuing_auto_fire
) {
	vector_t new_position = GS->ship.position;
	vector_t new_velocity;
	real_t new_speed_bonus = 1.0;
	real_t a;
	weapon_t* wpn = &GS->ship.weapons[WEAPON_ROUNDSHOT];

	int cost = FIRING_COST_ROUND_SHOT;

	if ((wpn->enabled) && (GS->current_resource >= cost)) {

		for( a = 1.1/(2*M_PI) ; a <= M_PI ; a *= 1.2 ) {

			new_velocity.x
				= cos(a-1/(2*M_PI)+M_PI/2)
				* LASER_SPEED_PLAYER
				* 0.75
			;
			new_velocity.y
				= sin(a-1/(2*M_PI)+M_PI/2)
				* LASER_SPEED_PLAYER
			;
			add_laser_beam(
				GS,
				OWNER_PLAYER_NO_RESPAWN,
				new_position,
				new_velocity,
				new_speed_bonus
			);
			new_velocity.x *= (-1);	// Fire into the other..
						// ..direction as well
			add_laser_beam(
				GS,
				OWNER_PLAYER_NO_RESPAWN,
				new_position,
				new_velocity,
				new_speed_bonus
			);
		}
		GS->current_resource -= cost;

		if (continuing_auto_fire) {
			wpn->fire_next_beam_us
				= PS->current_time_us
				+ AUTO_FIRE_INTERVAL
			;
			wpn->auto_fire_count++;
		}
		else {	// First shot
			wpn->fire_next_beam_us
				= PS->current_time_us
				+ AUTO_FIRE_DELAY
			;
			wpn->auto_fire_count = 1;
		}

		play_sound( GS->sounds.laser );
	}
	else {
		blink_weapon_hud( PS, wpn, DENIED_BLINK_DURATION );
		play_sound( GS->sounds.denied );
	}
}

/* continue_fire()
 * When a control is kept enabled (key held down) and a certain amount of time
 * passes by, the weapon will trigger again ("auto fire").
 */
void continue_fire( program_state_t* PS, game_state_t* GS, int weapon_nr )
{
	weapon_t* wpn = &GS->ship.weapons[weapon_nr];

	if (!wpn->enabled) {
		return;		// Prevent denied-sound from being played
	}

	if (!GS->ship.weapons[WEAPON_AUTOFIRE].enabled) {
		return;		// Auto fire not installed
	}

	if (
		(wpn->fire_next_beam_us < PS->current_time_us)
		&&
		(wpn->auto_fire_count < AUTO_FIRE_MAX_SHOTS)
	) {

		switch (weapon_nr) {
		case WEAPON_LASER_1:
			if (GS->current_resource
				> FIRING_COST_LASER_1 + AUTO_FIRE_MALUS
			) {
				start_fire( PS, GS, WEAPON_LASER_1, FM_AUTO );
				GS->current_resource -= AUTO_FIRE_MALUS;
			}
			break;

		case WEAPON_LASER_2:
			if (GS->current_resource
				> FIRING_COST_LASER_2 + AUTO_FIRE_MALUS
			) {
				start_fire( PS, GS, WEAPON_LASER_2, FM_AUTO );
				GS->current_resource -= AUTO_FIRE_MALUS;
			}
			break;

		case WEAPON_ROUNDSHOT:
			start_round_shot( PS, GS, FM_AUTO );
			break;
		}
	}
}


// SHIP MOVEMENT //////////////////////////////////////////////////////////////

void start_move( program_state_t* PS, game_state_t* GS, direction_t direction )
{
	thruster_state_t* ts = &(GS->ship.thruster_state);

	switch (direction) {
	case LEFT:	ts->left    = TRUE; 	break;
	case RIGHT:	ts->right   = TRUE; 	break;
	case FORWARD:	ts->forward = TRUE;

#if LIMIT_FORWARD_SPEEDING
			ts->forward_until_us
				= PS->current_time_us
				+ FORWARD_THRUSTING_MAX_TIME
			;
			ts->pay_after_us
				= PS->current_time_us
				+ FORWARD_THRUSTING_PAY_TIME
			;
#endif
						break;
	case BACK:	ts->back    = TRUE; 	break;
	}
}

void stop_move( program_state_t* PS, game_state_t* GS, direction_t direction )
{
	switch (direction) {
	case LEFT:	GS->ship.thruster_state.left    = FALSE;	break;
	case RIGHT:	GS->ship.thruster_state.right   = FALSE;	break;
	case FORWARD:	GS->ship.thruster_state.forward = FALSE;	break;
	case BACK:	GS->ship.thruster_state.back    = FALSE;	break;
	}
}


/******************************************************************************
* OTHER PLAYER RELATED FUNCTIONS
******************************************************************************/

/* player_takes_hit()
 * Calculates hit damage and reduces Resource. May end the game.
 */
void player_takes_hit(
	program_state_t* PS,
	game_state_t* GS,
	laser_beam_t* l
	)
{
	int k;
	ship_t* shp = &(GS->ship);
	sounds_t* snd = &(GS->sounds);

	if (GS->current_resource >= LASER_HIT_PENALTY_MIN) {
		// Player looses Resource

		k = GS->current_resource / 2;

		GS->current_resource
			-= (k < LASER_HIT_PENALTY_MIN)
			? LASER_HIT_PENALTY_MIN
			: k
		;

		if (calculate_hit_points(GS->current_resource) < NR_WEAPONS) {
			disable_best_weapon( PS, GS );
		}

		shp->indicate_hit_until_us
			= PS->current_time_us
			+ HIT_INDICATION_TIME_US
		;

		play_sound( snd->punch );

		if (GS->current_resource <= LASER_HIT_PENALTY_MIN) {
			play_sound( snd->alarm );
		}
	}
	else {
		// Player dies

		GS->current_resource = 0;

		PS->run_mode = RM_AFTER_LIFE;
		PS->after_life_start_us = get_time();

		add_explosion( PS, GS, shp->position );

		play_sound( snd->blast );
	}

}


//EOF
