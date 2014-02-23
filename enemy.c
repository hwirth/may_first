//enemy.c
/******************************************************************************
* ARTIFICIAL ENEMY INTELLIGENCE
******************************************************************************/

#include "enemy.h"
#include "main.h"
#include "game.h"
#include "player.h"
#include "formation.h"
#include "level_design.h"
#include "world.h"
#include "ui.h"


/******************************************************************************
* NEW ENEMY
******************************************************************************/

void determine_enemy_tier(
	game_state_t* GS,
	int tier,
	real_t* agressiveness,
	real_t* speed,
	real_t* score
	)
{
	switch (tier) {
		case MOTHERSHIP_TIER:
			*agressiveness = MOTHERSHIP_AGRESSIVENESS;
			*speed = 2.0;
			*score = MOTHERSHIP_SCORE;
			break;

		case TIER_3:
			*agressiveness = 6;
			*speed = 3.0;
			*score = 50;
			break;

		case TIER_2:
			*agressiveness = 3;
			*speed = 1.5;
			*score = 30;
			break;

		default:
			*agressiveness = 1;
			*speed = 1.0;
			*score = 10;
			break;
	}
}

enemy_t* find_inactive_enemy( game_state_t* GS )
{
	int i;
	enemy_t* e;

	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {

		e = &(GS->enemies[i]);

		if (e->active == FALSE) {

			return e;
		}
	}

	fprintf( stderr, "new_enemy(): Can't add enemy: all slots busy.\n" );
	return NULL;
}

/* add_enemy()
 * Creates a new entry in the  enemy[]  array.
 */
enemy_t* add_enemy(
	game_state_t* GS,
	int new_tier,
	vector_t new_position,
	vector_t new_velocity,
	formation_t* new_formation
	)
{
	real_t agressiveness;		// How often this enemy will shoot
	real_t speed;			// Base for calculating velocity
	real_t score;			//... RESOURCE gained upon kill

	enemy_t* e = find_inactive_enemy( GS );

	if (e != NULL) {

		determine_enemy_tier(
			GS,
			new_tier,
			&agressiveness,
			&speed,
			&score
		);

		get_tier_color(
			new_tier,
			&(e->color.R),
			&(e->color.G),
			&(e->color.B)
		);

		e->position = new_position;

		if ((speed > 0) && (vector_length(new_velocity) > 0)) {
			e->velocity
				= multiply_vector_scalar(
					unity_vector(new_velocity),
					speed
				)
			;
		}
		else {	e->velocity.x =
			e->velocity.y =
			e->velocity.z = 0;
		}

		e->formation = new_formation;

		if (new_formation == NULL) {

			e->current_rank = NULL;
			e->ai_mode = FREE;
		}
		else {
			e->current_rank = next_free_rank( e->formation );
			e->ai_mode = FORMATION;

			if (e->current_rank != NULL) {

				e->current_rank->occupied_by = e;
			}
		}
		e->target_rank = NULL;

		e->tier = new_tier;
		e->score = score;
		e->agressiveness = agressiveness;
		e->shoot_wait_until_us = 0;

		e->hit_points = get_tier_hitpoints( new_tier );

		e->active = TRUE;
		GS->nr_active_enemies[new_tier]++;
		GS->nr_active_enemies_total++;
	}

	return e;
}

void remove_enemy( game_state_t* GS, enemy_t* e )
{
	if (e->formation != NULL) {

		if (e->ai_mode == RANK_TRANSIT) {
			e->target_rank->occupied_by = NULL;
		}
		else {
			e->current_rank->occupied_by = NULL;
		}
	}

	e->active = FALSE;
	GS->nr_active_enemies[e->tier]--;
	GS->nr_active_enemies_total--;
}


/******************************************************************************
* ENEMY SPECIFIC EVENT HANDLERS
******************************************************************************/

/* enemy_takes_hit()
 * Calculates damage (reduces the enemy's hit points), may disable that enemy.
 * Increases RESOURCE accordingly.
 */
void enemy_takes_hit(
	program_state_t* PS,
	game_state_t* GS,
	enemy_t* e,
	laser_beam_t* l
	)
{
	if (l->owner < 0) {		// Player
		e->hit_points--;

#if ENEMIES_CHANGE_DIRECTION_ON_HIT
		e->velocity.x *= (-1);	// Change direction
		if (e->formation != NULL) {
			formation_turn_around( GS, e->formation );
		}
#endif
	}

	if (e->hit_points > 0) {
		e->indicate_hit_until_us
			= PS->current_time_us
			+ HIT_INDICATION_TIME_US
		;
	}
	else if (l->owner < 0) {	// Player

		GS->score.current += e->score * BONUS_FACTOR_SCORE;

		add_explosion( PS, GS, e->position );

		add_bonus_bubble(
			PS,
			GS,
			e->position,
			e->color,
			e->tier,
			e->score
#if CHEAT_MODE
				* CHEAT_RESOURCE_FACTOR
#endif

		);

		remove_enemy( GS, e );

		// Replace the killed enemy
		//... -->level_design.c
		if (l->owner != OWNER_PLAYER_NO_RESPAWN) {
			//... now distance triggerd:
			//...new_formation( GS, e->agressiveness );
		}

		// Reset ENEMY BEYOND counter
		GS->add_enemy_beyond_y = calculate_enemy_beyond_y( GS );
		GS->nr_warp_enemies = MIN_WARP_ENEMIES;	// Reset spawn amount

		play_sound( GS->sounds.hit );
	}
}


/******************************************************************************
* ENEMY AI
******************************************************************************/

/* enemy_ai_contol()
 * Initiates enemy actions like attacking, changing course and general
 * "thinking". Is called before actual movement for the current frame is
 * executed.
 */
void enemy_ai_control( program_state_t* PS, game_state_t* GS, int enemy_nr )
{
	real_t dx, dy, distance;
	vector_t new_velocity;
	vector_t new_position;
	real_t tier_shoot_factor;
	real_t new_speed_bonus = 1.0;
	enemy_t* e = &GS->enemies[enemy_nr];


	// Toggle x-velocity

	vector_t pos = e->position;
	vector_t vel;

	switch (e->ai_mode) {
	case FORMATION:		// Fall through
	case RANK_TRANSIT:
		vel = e->formation->velocity;
		break;
	default:
		vel = e->velocity;
	}

	if ( (pos.x + vel.x*PS->tick_fraction_s < FIELD_MIN_X)
	||   (pos.x + vel.x*PS->tick_fraction_s > FIELD_MAX_X)
	) {
		if (fabs(pos.x + vel.x) > fabs(pos.x)) {
			switch (e->ai_mode) {
			case FORMATION:		// Fall through
			case RANK_TRANSIT:
				formation_turn_around( GS, e->formation );
				break;
			default:
				e->velocity.x *= (-1);
			}
		}
	}


	// Shoot interval reached?

	if ( (e->shoot_wait_until_us < PS->current_time_us)
	&&   (e->position.y > GS->ship.position.y - AI_SHOOT_BEHIND_DISTANCE)
	&&   (e->position.y - GS->ship.position.y >= AI_MIN_SHOOT_DISTANCE)
	&&   (e->position.y - GS->ship.position.y <= AI_MAX_SHOOT_DISTANCE)
#if FREEZE_ENEMY_WEAPONS
	&&   FALSE
#endif
	) {
		new_position = e->position;
		new_velocity.x =
		new_velocity.y =
		new_velocity.z = 0.0;

		switch (e->agressiveness) {
		case MOTHERSHIP_AGRESSIVENESS:
			// Do aim
			dx = GS->ship.position.x - e->position.x;
			dy
				= GS->ship.position.y
				+ MOTHERSHIP_PRE_AIM_OFFSET
				- e->position.y
			;
			distance = sqrt( dx*dx + dy*dy );

			new_velocity.x
				= dx
				/ distance
				* LASER_SPEED_MOTHERSHIP
			;
			new_velocity.y
				= dy
				/ distance
				* LASER_SPEED_MOTHERSHIP
			;
			tier_shoot_factor = MOTHERSHIP_SHOOT_FACTOR;
			break;
		default:
			new_velocity.y = -LASER_SPEED_ENEMY;
			tier_shoot_factor = ENEMY_TIER_SHOOT_FACTOR;
		}

		add_laser_beam(
			GS,
			enemy_nr,
			new_position,
			new_velocity,
			new_speed_bonus
		);
		//...play_sound( laser );

		// Calculate timestamp for next shot
		e->shoot_wait_until_us
			= PS->current_time_us
			+ ENEMY_BASE_SHOOT_INTERVAL_US
			/ sqrt(
				e->agressiveness
				* tier_shoot_factor
			)
			* get_tier_hitpoints( e->tier )
			/ e->hit_points
		;
	}

}

/* simulate_enemy_ai()
 * called in  advance_simulation() ,  world.c .
 */
void simulate_enemy_ai( program_state_t* PS, game_state_t* GS )
{
	int i;

	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {
		if (GS->enemies[i].active) {
			enemy_ai_control( PS, GS, i );
		}
	}
}


/******************************************************************************
* AI ADVANCE POSITION
******************************************************************************/

/* advance_enemy_free()
 * Move enemy according to its own velocity value.
 */
void advance_enemy_free(
	program_state_t* PS,
	enemy_t* enemy
	)
{
	enemy_t* e = enemy;

	e->position.x += e->velocity.x * PS->tick_fraction_s;
	e->position.y += e->velocity.y * PS->tick_fraction_s;
	e->position.z += e->velocity.z * PS->tick_fraction_s;
}

/* advance_enemy_formation()
 * Move enemy according to current formation data.
 */
void advance_enemy_formation(
	program_state_t* PS,
	enemy_t* enemy
	)
{
	enemy_t* e = enemy;
	formation_t* f = e->formation;

	vector_t rank_offset;

	rank_offset.x = e->current_rank->position.x;
	rank_offset.y = e->current_rank->position.y;
	rank_offset.z = e->position.z;

	e->position = add_vector(
		add_vector(
			f->position,
			rank_offset
		),
		multiply_vector_scalar(
			f->velocity,
			PS->tick_fraction_s
		)
	);
}

/* ai_rank_transit()
 * Advance position en route to another rank in the current formation
 */
void advance_enemy_rank_transit(
	program_state_t* PS,
	enemy_t* enemy
	)
{
	enemy_t* e = enemy;
	formation_t* f = e->formation;

	vector_t path, current_offset;
	real_t progress;

	path = subtract_vector(
		e->target_rank->position,
		e->current_rank->position
	);

	progress
		= (real_t) (PS->game_time_us - e->rank_change_start_us)
		/ RANK_TRANSIT_TIME
	;

	if (progress < 1.0) {	// Less than 100% travelled?

		current_offset = multiply_vector_scalar(
			path,
			progress
		);

		e->position = add_vector(
			add_vector(
				f->position,
				e->current_rank->position
			),
			current_offset
		);
	}
	else {	// Target position reached

		e->current_rank = e->target_rank;
		e->target_rank = NULL;

		// Recalculate new position from scratch
		// (relative to the formation) to prevent
		// rounding errors
		e->position = add_vector(
			e->formation->position,
			e->current_rank->position
		);

		e->ai_mode = FORMATION;
	}

	e->position.x += f->velocity.x * PS->tick_fraction_s;
	e->position.y += f->velocity.y * PS->tick_fraction_s;
	e->position.z += f->velocity.z * PS->tick_fraction_s;
}

/* advance_enemy_position()
 */
void advance_enemy_position(
	program_state_t* PS,
	game_state_t* GS,
	enemy_t* enemy
	)
{
	ship_t* s = &(GS->ship);
	enemy_t* e = enemy;

	switch (e->ai_mode) {
	case FREE:		advance_enemy_free( PS, e );		break;
	case RANK_TRANSIT:	advance_enemy_rank_transit( PS, e );	break;
	case FORMATION:		advance_enemy_formation( PS, e );	break;
	case FOLLOW:
	case CRASH_INTO:
	case ORBIT:
	default:
		error_quit("ai_advance_position(): Funny ai_mode");
	}

	normalize_position_y( GS, &(enemy->position) );

	// Collision with ship
	if (detect_collision(
		PS->tick_fraction_s,
		e->position, e->velocity,
		s->position, s->velocity,
		enemy_size(GS, e)
		)
	) {
		remove_enemy( GS, e );

		add_explosion( PS, GS, s->position );
		add_explosion( PS, GS, e->position );

		GS->current_resource = 0;

		PS->after_life_start_us = get_time();
		PS->run_mode = RM_AFTER_LIFE;

		play_sound( GS->sounds.blast );
	}
}

/* advance_enemies()
 */
void advance_enemies( program_state_t* PS, game_state_t* GS )
{
	int i;

	for( i = 0 ; i < MAX_FORMATIONS ; i++ ) {
		if (GS->formations[i].nr_ranks > 0) {
			advance_formation( PS, GS, &(GS->formations[i]) );
		}
	}

	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {

		if (GS->enemies[i].active) {
			advance_enemy_position( PS, GS, &(GS->enemies[i]) );
		}
	}
}


//EOF
