//enemy.h
/******************************************************************************
* ARTIFICIAL ENEMY INTELLIGENCE
*******************************************************************************
* WEAKNESS
* - Moving medium  --> easy to aim
* - No situaltional awareness (evading)
*
* DEFENSIVE
* - Standing still --> hard to aim at
* - Moving fast    --> hard to hit
* - Hitpoints, Recharging
* - Size (bigger is easier to hit)
* - Changing course sporadically
* - Evading
* - Hyperjump
*
* AGRESSIVE
* - Takes aim
* - Follows or reacts otherwise to the player
* - Slowly turning to a certain heading, then rushing forward
* - Kamikaze
* - Special attack: Controls mirrored
******************************************************************************/
#ifndef ENEMY_CONTROL_H_PARSED
#define ENEMY_CONTROL_H_PARSED


#include "main.h"
#include "game.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////

enemy_t* add_enemy(
	game_state_t* GS,
	int new_tier,
	vector_t new_position,
	vector_t new_velocity,
	formation_t* new_formation
);
void remove_enemy( game_state_t* GS, enemy_t* e );

void enemy_takes_hit(
	program_state_t* PS,
	game_state_t* GS,
	enemy_t* e,
	laser_beam_t* l
);

void simulate_enemy_ai( program_state_t* PS, game_state_t* GS );

void ai_advance_position(
	program_state_t* PS,
	game_state_t* GS,
	enemy_t* enemy
);

void advance_enemies( program_state_t* PS, game_state_t* GS );


#endif
//EOF
