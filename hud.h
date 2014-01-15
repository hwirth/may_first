//hud.h
/******************************************************************************
* DRAW HUD
******************************************************************************/
#ifndef DRAW_HUD_H_PARSED
#define DRAW_HUD_H_PARSED


#include "main.h"
#include "game.h"


// DEFINES ////////////////////////////////////////////////////////////////////

#define ALIGN_LEFT	1
#define ALIGN_CENTER	2
#define ALIGN_RIGHT	4
#define ALIGN_TOP	8
#define ALIGN_MIDDLE	16
#define ALIGN_BOTTOM	32


// TYPEDEFS ///////////////////////////////////////////////////////////////////

typedef unsigned int alignment_t;


// PROTOTYES //////////////////////////////////////////////////////////////////

void hud_printf(
	program_state_t* PS,
	int x,
	int y,
	alignment_t alignment,
	int color,
	char* format_string,
	...
);

void draw_hud( program_state_t* PS, game_state_t* GS );
void draw_debug( program_state_t* PS, game_state_t* GS );


#endif
//EOF
