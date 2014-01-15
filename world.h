//world.h
/******************************************************************************
* WORLD SIMULATION
******************************************************************************/
#ifndef WORLD_SIMULATION_H_PARSED
#define WORLD_SIMULATION_H_PARSED


#include "main.h"
#include "game.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////

void remove_all_objects( game_state_t* GS );
void disable_weapons( game_state_t* GS );

void remove_laser_beam( game_state_t* GS, laser_beam_t* l );
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
);

void remove_explosion( game_state_t* GS, explosion_t* e );
void add_explosion(
	program_state_t* PS,
	game_state_t* GS,
	vector_t new_position
);

void remove_bonus_bubble( game_state_t* GS, bonus_bubble_t* b );
void add_bonus_bubble(
	program_state_t* PS,
	game_state_t* GS,
	vector_t new_position,
	color_t new_color,
	int new_tier,
	int new_resource
);

bool_t detect_collision(
	real_t tick_fraction_s,
	vector_t position1, vector_t velocity1,
	vector_t position2, vector_t velocity2,
	real_t distance
);

void advance_simulation( program_state_t* PS, game_state_t* GS );


#endif
//EOF
