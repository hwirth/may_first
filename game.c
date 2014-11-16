//game.c
/******************************************************************************
* GAME LOGIC
*******************************************************************************
* Functions in this module are used commonly throughout the program.
* These functions perform certain calculations or provide generic actions and
* game control.
******************************************************************************/

#include <stdlib.h>		// rand()
#include <stdio.h>
#include <math.h>
#include <SDL/SDL_mixer.h>

#include "game.h"
#include "main.h"
#include "level_design.h"
#include "world.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"	// PLAY_SOUNDS


// ACTIONS ////////////////////////////////////////////////////////////////////

void blink_weapon_hud(
	program_state_t* PS,
	weapon_t* weapon,
	microtime_t blink_duration
	)
{
	weapon->blink_start_us = PS->current_time_us;
	weapon->blink_until_us = PS->current_time_us + blink_duration;
}

void disable_best_weapon( program_state_t* PS, game_state_t* GS )
{
	ship_t* s = &(GS->ship);

	if (s->weapons[WEAPON_ROUNDSHOT].enabled) {
		s->weapons[WEAPON_ROUNDSHOT].enabled = FALSE;
		blink_weapon_hud(
			PS, &s->weapons[WEAPON_ROUNDSHOT],
			ACHIEVEMENT_LOST_BLINK_DURATION
		);
	}
	else if (s->weapons[WEAPON_LASER_2].enabled) {
		s->weapons[WEAPON_LASER_2].enabled = FALSE;
		blink_weapon_hud(
			PS, &s->weapons[WEAPON_LASER_2],
			ACHIEVEMENT_LOST_BLINK_DURATION
		);
	}
	else if (s->weapons[WEAPON_AUTOFIRE].enabled) {
		s->weapons[WEAPON_AUTOFIRE].enabled = FALSE;
		blink_weapon_hud(
			PS, &s->weapons[WEAPON_AUTOFIRE],
			ACHIEVEMENT_LOST_BLINK_DURATION
		);
	}

#if PLAY_COMPUTER_VOICE
	play_sound( GS->sounds.computer_weaponlost );
#endif
}


// CALCULATIONS ///////////////////////////////////////////////////////////////

real_t enemy_size( game_state_t* GS, enemy_t* enemy )
{
	return ENEMY_SIZE_FACTOR
		+ ENEMY_SIZE_FACTOR
		* (enemy->hit_points - 1) / 4
	;
}

real_t bubble_size( bonus_bubble_t* b )
{
	real_t resource
		= b->resource
#if CHEAT_MODE
		/ CHEAT_RESOURCE_FACTOR
#endif
	;

	const real_t a = sqrt(resource) * sqrt(MOTHERSHIP_SCORE);
	const real_t gain = BONUS_BUBBLE_MAX_RADIUS - BONUS_BUBBLE_MIN_RADIUS;
	real_t size_factor = a / MOTHERSHIP_SCORE;

	return BONUS_BUBBLE_MIN_RADIUS + gain * size_factor;
}

int get_tier_color( int tier, real_t* R, real_t* G, real_t* B )
{
	int color;

	switch (tier) {
		case TIER_1:	*R = 0.1;  *G = 0.1;  *B = 1.0;	break;
		case TIER_2:	*R = 0.1;  *G = 1.0;  *B = 0.1;	break;
		case TIER_3:	*R = 1.0;  *G = 0.1;  *B = 0.1;	break;
		case TIER_4:	*R = 1.0;  *G = 1.0;  *B = 0.1;	break;
		default:	*R = *G = *B = 0.5;
	}

	color	= ((int)(*R * 255) << 16)
		+ ((int)(*G * 255) << 8)
		+ ((int)(*B * 255) << 0)
	;

	return color;
}

real_t get_tier_hitpoints( int tier )
{
	real_t hit_points = 0;

	switch (tier) {
		case TIER_1:	hit_points = 1;		break;
		case TIER_2:	hit_points = 3;		break;
		case TIER_3:	hit_points = 6;		break;
		case TIER_4:	hit_points = MOTHERSHIP_AGRESSIVENESS;
	}

	return hit_points;
}

real_t level_length( game_state_t* GS )
{
	return FIELD_HEIGHT/4 + 0.1 * (real_t) GS->current_level;
}

void normalize_position_y( game_state_t* GS, vector_t* position )
{
	ship_t* s = &(GS->ship);

	// General offset (*) If 0, respawn just below visible horizon

	real_t enter_y
		= s->position.y
		+ FIELD_HEIGHT * 0.15   // General offset (*)
		+ level_length( GS )    // Increasingly larger levels
	;

	// Reposition enemies that are far behind the player
	if (position->y - s->position.y < -FIELD_HEIGHT/8) {

		position->y = enter_y;
	}
}

/* calculate_hit_points()
 * Returns the number of times you can divide RESOURCE by 2 until you are
 * below or at 50, plus 1 (If you have some RESOURCE below 50, you still have
 * one hit point).
 */
int calculate_hit_points( real_t resource )
{
	real_t f;

	if (resource == 0) {
		return 0;
	}

	f = ceil(
		(1 + resource - LASER_HIT_PENALTY_MIN)
		/ LASER_HIT_PENALTY_MIN
	);

	return
		1 +
		(	(f >= 1)
			? 1 + (int) log2( f )
			: 0
		)
	;
}

/* calculate_hit_ratio()
 * Calculates the ratio between fired shots and missed ones, disregarding beams
 * that are currently en route.
 */
real_t calculate_hit_ratio( game_state_t* GS )
{
	real_t evaluated_shots = GS->shots_fired - GS->shots_en_route;

	if (evaluated_shots == 0) {
		return 0;
	}
	else {	return 1.0 - (real_t) GS->shots_missed / evaluated_shots;
	}
}


// CONTROLLED RANDOM //////////////////////////////////////////////////////////

/* rand_int()
 * http://www.christianpinder.com/articles/pseudo-random-number-generation/
 * Repeatedly called, a pseudo random number sequence is generated. The
 * sequence will always be the same, if started with the same  random_seed
 * value.
 */
int rand_int( int* random_seed )
{
	unsigned int hi, lo;

	hi = 16807 * (*random_seed >> 16);
	lo = 16807 * (*random_seed & 0xFFFF);
	lo += (hi & 0x7FFF) << 16;
	lo += hi >> 15;

	if (lo > 2147483647) {
		lo -= 2147483647;
	}
	*random_seed = lo;

	return *random_seed;
}

// MAIN GAME CONTROL //////////////////////////////////////////////////////////

// SCORE //////////////////////////////////////////////////////////////////////

/* When changing the algorithm, also check  hud_score_summary()  in  hud.c
 */
int calculate_total_score( program_state_t* PS, game_state_t* GS )
{
	const real_t min_speed = SPEED_Y / 2;	// Allow some back thrusting
	const real_t max_speed = SPEED_Y * FORWARD_SPEED_FACTOR;

	real_t s_elapsed = PS->game_time_us / 1000000.0;
	real_t average_speed = GS->ship.position.y / s_elapsed;

	// Normalize speeds to  min_speed :
	real_t relative_speed = average_speed - min_speed;
	real_t relative_max   = max_speed - min_speed;

	real_t speed_ratio
		= fmax(	0,
			fmin( 1, relative_speed / relative_max )
		)
	;

	score_info_t* si = &(GS->score);

	si->distance      = GS->ship.position.y / 100;
	si->best_resource = GS->best_resource;
	si->hit_ratio     = calculate_hit_ratio(GS) * 100.0;
	si->speed         = speed_ratio * 100.0;

	return	  si->current
		+ si->current * si->hit_ratio / 100
		+ si->current * si->speed / 100
		+ si->best_resource * BONUS_FACTOR_BEST_RESOURCE
#ifdef DISABLED_CODE
		+ si->distance      * BONUS_FACTOR_DISTANCE
		+ GS->enemies_killed * BONUS_FACTOR_ENEMIES /
			( (si->distance == 0) ? 1 : si->distance )
#endif
	;
}

void load_highscore( game_state_t* GS )
{
	FILE* fh;
	char buffer[255];

	if ( (fh = fopen(HIGHSCORE_FILENAME, "r")) ) {

		fscanf( fh, "%s", buffer );

		if (strcmp(buffer, PROGRAM_VERSION) == 0) {
			fscanf( fh, "%d", &(GS->score.high_score_level) );
			fscanf( fh, "%d", &(GS->score.high_score) );
		}

		fclose( fh );
	}
}

void save_highscore( program_state_t* PS, game_state_t* GS )
{
#if !CHEAT_MODE
	FILE* fh;
	int total_score = calculate_total_score( PS, GS );

	if (total_score > GS->score.high_score) {

		GS->score.high_score = total_score;
		GS->score.high_score_level = GS->current_level;

		if ( (fh = fopen(HIGHSCORE_FILENAME, "w")) ) {
			fprintf(
				fh,
				PROGRAM_VERSION"\n%d\n%d\n",
				GS->score.high_score_level,
				GS->score.high_score
			);
			fclose( fh );
		}
	}
#endif
}


/* reset_game()
 * Prepares a new game and sets  run_mode = RM_RUNNING
 */
void reset_game( program_state_t* PS, game_state_t* GS )
{
PS->highest_frame_time	= 0;
PS->lowest_frame_time	= 99999999;

	save_highscore( PS, GS );
	remove_all_objects( GS );	// Deactivate beams, explosions, ...
	disable_weapons( GS );

	GS->camera.angle    = INITIAL_CAMERA_ANGLE;
	GS->camera.distance = INITIAL_CAMERA_DISTANCE;

	PS->game_start_us = PS->current_time_us;
	PS->next_second_us = PS->game_start_us + 1000000;
	PS->game_time_us = 0;

	prepare_first_level( PS, GS );	// Create initial enemies

	PS->run_mode = RM_RUNNING;
}

/* toggle_pause()
 * Halts the game by adjusting  game_start_us  when coming back from pause
 */
void toggle_pause( program_state_t* PS )
{
	if (PS->run_mode == RM_RUNNING) {
		PS->run_mode = RM_PAUSE;
		PS->pause_since_us = get_time();
	}
	else if (PS->run_mode == RM_PAUSE) {
		PS->run_mode = RM_RUNNING;
		PS->game_start_us += get_time() - PS->pause_since_us;
	}
}


//EOF
