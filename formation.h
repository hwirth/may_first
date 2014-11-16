//formation.h
/******************************************************************************
* FORMATIONS
******************************************************************************/
#ifndef FORMATION_H_PARSED
#define FORMATION_H_PARSED


#include "game.h"
#include "formation.h"


int create_formation(
	game_state_t* GS,
	formation_t* formation,
	int top_tier,
	int formation_width,
	int formation_index,
	int nr_formations,
	real_t offset_y
);

void create_formation_enemies(
	game_state_t* GS,
	formation_t* formation,
	int top_tier,
	int amount
);

void formation_turn_around( game_state_t* GS, formation_t* formation );
formation_rank_t* next_free_rank( formation_t* formation );

void advance_formation(
	program_state_t* PS,
	game_state_t* GS,
	formation_t* formation
);


#endif
//EOF
