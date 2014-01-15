//player.h
/******************************************************************************
* PLAYER CONTROL
*******************************************************************************
* Functions in this module interface the input coming from the user (keyboard,
* joystick, etc.) to the game (player's ship). Actions may incluide steering,
* firing and selecting weapons, using the board computer (targeting), etc.
* These functions are typically called from  ui.c .
******************************************************************************/
#ifndef PLAYER_CONTROL_H_PARSED
#define PLAYER_CONTROL_H_PARSED


#include "main.h"
#include "game.h"


// TYPEDEFS ///////////////////////////////////////////////////////////////////

typedef enum direction_e {		// player.c  <--  ui.c
	LEFT, RIGHT, FORWARD, BACK
} direction_t;


// PROTOTYPES /////////////////////////////////////////////////////////////////

void start_fire(
	program_state_t* PS,
	game_state_t* GS,
	int weapon_nr,
	bool_t continuing_auto_fire
);
void start_round_shot(
	program_state_t* PS,
	game_state_t* GS,
	bool_t continuing_auto_fire
);

void continue_fire( program_state_t* PS, game_state_t* GS, int weapon_nr );

void start_move(
	program_state_t* PS,
	game_state_t* GS,
	direction_t direction
);

void stop_move( program_state_t* PS, game_state_t* GS, direction_t direction );


void player_takes_hit(
	program_state_t* PS,
	game_state_t* GS,
	laser_beam_t* l
);


#endif
//EOF
