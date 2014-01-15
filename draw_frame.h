//draw_frame.h
/******************************************************************************
* DRAW FRAME
******************************************************************************/
#ifndef DRAW_FRAME_H_PARSED
#define DRAW_FRAME_H_PARSED


#include "main.h"
#include "game.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////

void init_opengl( program_state_t* PS );

void draw_frame( program_state_t* PS, game_state_t* GS );
void draw_exit_frame( program_state_t* PS, game_state_t* GS );


#endif
//EOF
