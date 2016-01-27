//ui.h
/******************************************************************************
* USER INTERFACE
******************************************************************************/
#ifndef USER_INTERFACE_H_PARSED
#define USER_INTERFACE_H_PARSED


#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "main.h"
#include "game.h"


// OPTIONS ////////////////////////////////////////////////////////////////////

#define START_IN_FULLSCREEN      (! TRUE)
#define PLAY_SOUNDS              TRUE
#define PLAY_MUSIC               TRUE
#define PLAY_COMPUTER_VOICE      TRUE

#define VOLUME_STEPS             (MIX_MAX_VOLUME / 16)

#define MOUSE_CURSOR_VISIBLE_US  500000


// DEFINES ////////////////////////////////////////////////////////////////////

#define VOLUME_FX                1
#define VOLUME_MUSIC             2
#define VOLUME_ALL               (VOLUME_FX | VOLUME_MUSIC)

#define WINDOW_CAPTION           "May First"

#define INITIAL_WINDOW_WIDTH     640
#define INITIAL_WINDOW_HEIGHT    480
#define SCREEN_BPP               32

#define FONT_FILENAME            "./media_files/DejaVuSansMono.ttf"
// Ubuntu, License: Unknown

#define FONT_SIZE                22       //20 Will be adjusted in init_font()

#define BUTTON_DOWN              TRUE
#define BUTTON_UP                FALSE


#define ERROR_SDL_TTF_OPENFONT_RETURNED_NULL    "TTF_OpenFont() returned NULL"
#define ERROR_SDL_TTF_RENDERTEXT_RETURNED_NULL  "TTF_RenderText() returned NULL"
#define ERROR_SDL_IMG_LOAD_RETURNED_NULL        "IMG_Load() returned NULL"


// PROTOTYPES /////////////////////////////////////////////////////////////////

void init_sdl( program_state_t* PS, game_state_t* GS );
void init_sound( program_state_t* PS, game_state_t* GS );
void init_font( program_state_t* PS );

void play_sound( Mix_Chunk* sound );
void play_music( Mix_Music* music );

void hide_cursor( void );
void show_cursor( void );

void process_event_queue( program_state_t* PS, game_state_t* GS );


#endif
//EOF
