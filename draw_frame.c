//draw_frame.c
/******************************************************************************
* DRAW FRAME
*******************************************************************************
* The main loop calls  draw_frame()  repeatedly in order to update the screen
* contents. Depending on PS->run_mode, different sub-routines are used to
* render the according graphics.
*
* Currently there are 3 different screens:
* - Intro (Credit Screen)
* - Game  (Including pause "menu" and after-death score overview ("main menu")
* - Exit  This screen helps hiding the odd pause, when SDL stuff is unloaded
*
* If debugging is enabled, draw_debug() (hud.c) will show various statistics.
******************************************************************************/

#include <malloc.h>     // Used for statistics...
//#include <locale.h>   // ...in Draw_HUD()
#include <math.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>

#include "draw_frame.h"
#include "main.h"
#include "gl_helpers.h"
#include "hud.h"
#include "intro.h"
#include "scene.h"
#include "game.h"


// OPEN GL INITIALIZATION /////////////////////////////////////////////////////

/* init_opengl()
 * Application specific preparations like loading the background image and
 * sending it to the GPU
 */
void init_opengl( program_state_t* PS )
{
	int w, h;
	textures_t* t = &(PS->textures);

	t->background = load_texture( BACKGROUND_FILENAME, &w, &h );

	if (t->background == 0) {
		error_quit( "load_texture() returned NULL" );
	}

	t->digits = load_texture( DIGITS_FILENAME, &w, &h );
	if (t->digits == 0) {
		error_quit( "load_texture() returned NULL" );
	}

//...SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//...SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

}


// EXIT SCREEN ////////////////////////////////////////////////////////////////

/* draw_exit_frame()
 * Shows something, while SDL unloads.
 */
void draw_exit_frame( program_state_t* PS, game_state_t* GS )
{
	int w = PS->window_width;
	int h = PS->window_height;

	glViewport( 0, 0, w, h );

	glClearColor( 0.0f, 0.0f, 0.1f, 1.0f ); // Background black and opaque
	glClearDepth( 1.0f );                   // Background depth to farthest
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );          // Setup 2D mode
	glLoadIdentity();
	gluOrtho2D(
		0, w,                           // Let coordinates match..
		0, h                            // ..screen resolution
	);
	glMatrixMode( GL_MODELVIEW );
	glDisable( GL_DEPTH_TEST );

	hud_printf(
		PS,
		w/2,
		h/2,
		ALIGN_CENTER,
		0xFFFFFF,
		"http://harald.ist.org/"
	);
}


// DRAW FRAME /////////////////////////////////////////////////////////////////

/* draw_frame()
 * Gets called from the main loop once every frame is to be drawn.
 * Will call the drawing function related to the current run mode.
 */
void draw_frame( program_state_t* PS, game_state_t* GS )
{
	switch (PS->run_mode) {
		case RM_INTRO:       draw_intro_frame( PS );     break;
		case RM_MAIN_MENU:   // fall through
		case RM_RUNNING:     // fall through
		case RM_PAUSE:       // fall through
		case RM_AFTER_LIFE:  draw_game_frame( PS, GS );  break;
		case RM_EXIT:        draw_exit_frame( PS, GS );  break;
	}

	// Show debug info
	if (PS->debug) {
		draw_debug( PS, GS );
	}

	SDL_GL_SwapBuffers();        // Wait for VSYNC and update screen

	//...log_opengl_errors();
}

//EOF
