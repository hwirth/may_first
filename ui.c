//ui.c
/******************************************************************************
* USER INTERFACE
*******************************************************************************
* This module encapsulates everything needed to work with the operating system
* (desktop), like creating a window, handling input, etc.
******************************************************************************/
#include <time.h>

#include <SDL/SDL.h>

#include "ui.h"
#include "main.h"
#include "gl_helpers.h"
#include "draw_frame.h"
#include "world.h"
#include "game.h"
#include "level_design.h"
#include "player.h"


// INITIALIZATION /////////////////////////////////////////////////////////////

void init_sdl( program_state_t* PS, game_state_t* GS )
{
	int w = INITIAL_WINDOW_WIDTH;
	int h = INITIAL_WINDOW_HEIGHT;

	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_WM_SetCaption( WINDOW_CAPTION, NULL );

	SDL_JoystickEventState( SDL_ENABLE );
	PS->joystick = SDL_JoystickOpen( 0 );

#if START_IN_FULLSCREEN
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	w = info->current_w;
	h = info->current_h;
#else
	w = INITIAL_WINDOW_WIDTH;
	h = INITIAL_WINDOW_HEIGHT;
#endif

	PS->window_width  = w;
	PS->window_height = h;

	PS->screen = SDL_SetVideoMode(
		w, h,
		SCREEN_BPP,
		PS->screen_flags
	);
	if (PS->screen == NULL) {
		error_quit( ERROR_SDL_IMG_LOAD_RETURNED_NULL );
	}

	PS->mouse.visible_until_us = get_time() + MOUSE_CURSOR_VISIBLE_US;
}

void init_sound( program_state_t* PS, game_state_t* GS )
{
#if (PLAY_SOUNDS || PLAY_MUSIC)
	sounds_t* snd = &(GS->sounds);
#endif
#if PLAY_SOUNDS
	//if (Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) {
	if (Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) == -1 ) {
		error_quit( "Mix_OpenAudio() returned NULL" );
	}

	snd->laser  = NULL;   //... do I really need these initializations?
	snd->hit    = NULL;
	snd->punch  = NULL;
	snd->blast  = NULL;
	snd->denied = NULL;
	snd->alarm  = NULL;

	snd->computer_autofire   = NULL;
	snd->computer_doubleshot = NULL;
	snd->computer_roundshot  = NULL;
	snd->computer_danger     = NULL;
	snd->computer_weaponlost = NULL;

	snd->music  = NULL;

	snd->laser  = Mix_LoadWAV( LASER_WAV  );
	snd->hit    = Mix_LoadWAV( HIT_WAV    );
	snd->punch  = Mix_LoadWAV( PUNCH_WAV  );
	snd->blast  = Mix_LoadWAV( BLAST_WAV  );
	snd->denied = Mix_LoadWAV( DENIED_WAV );
	snd->alarm  = Mix_LoadWAV( ALARM_WAV  );

	snd->computer_autofire   = Mix_LoadWAV( COMPUTER_AUTOFIRE_WAV   );
	snd->computer_doubleshot = Mix_LoadWAV( COMPUTER_DOUBLESHOT_WAV );
	snd->computer_roundshot  = Mix_LoadWAV( COMPUTER_ROUNDSHOT_WAV  );
	snd->computer_danger     = Mix_LoadWAV( COMPUTER_DANGER_WAV     );
	snd->computer_weaponlost = Mix_LoadWAV( COMPUTER_WEAPONLOST_WAV );

	snd->blub   = Mix_LoadWAV( BLUB_WAV   );

	if (!snd->laser || !snd->hit || !snd->punch
	|| !snd->blast || !snd->denied || !snd->alarm) {
		error_quit( "One of the sounds could not be loaded" );
	}
#endif

#if PLAY_MUSIC
	snd->music = Mix_LoadMUS( GAME_MUSIC );

	if (snd->music == NULL) {
		error_quit( "The background music could not be loaded" );
	}
#endif

	PS->volume_fx    = MIX_MAX_VOLUME / 3;
	PS->volume_music = MIX_MAX_VOLUME / 2;

#if (PLAY_SOUNDS || PLAY_MUSIC)
	Mix_Volume( -1, PS->volume_fx );
	Mix_VolumeMusic( PS->volume_music );
#endif
}

void init_font( program_state_t* PS )
{
	PS->font_size = FONT_SIZE * PS->window_height / 1050;
	PS->line_height = PS->font_size + 2;

	TTF_Init();
	PS->font = TTF_OpenFont( FONT_FILENAME, PS->font_size );
	if (PS->font == NULL) {
		error_quit( ERROR_SDL_TTF_OPENFONT_RETURNED_NULL );
	}
}


/******************************************************************************
* SDL HELPERS
******************************************************************************/

void play_sound( Mix_Chunk* sound )
{
#if PLAY_SOUNDS
	if (Mix_PlayChannel( -1, sound, 0 ) == -1) {
		//...error_quit( "play_sound: Mix_PlayChannel() returned -1" );
	}
#endif
}

void play_music( Mix_Music* music )
{
#if PLAY_MUSIC
	if (Mix_PlayingMusic() == 0) {
		if (Mix_PlayMusic(music, -1) == -1) {
			error_quit( "Mix_PlayMusic() returned -1" );
		}
	}
#endif
}

void toggle_music()
{
	if (Mix_PausedMusic() == 1) {
		Mix_ResumeMusic();
	}
	else {
		Mix_PauseMusic();
	}
}

void volume_up( program_state_t* PS, int channel )
{
	if (channel & VOLUME_FX) {
		PS->volume_fx += VOLUME_STEPS;
		if (PS->volume_fx > MIX_MAX_VOLUME) {
			PS->volume_fx = MIX_MAX_VOLUME;
		}
		Mix_Volume( -1, PS->volume_fx );
	}
	if (channel & VOLUME_MUSIC) {
		PS->volume_music += VOLUME_STEPS;
		if (PS->volume_music > MIX_MAX_VOLUME) {
			PS->volume_music = MIX_MAX_VOLUME;
		}
		Mix_VolumeMusic( PS->volume_music );
	}
}

void volume_down( program_state_t* PS, int channel )
{
	if (channel & VOLUME_FX) {
		PS->volume_fx -= VOLUME_STEPS;
		if (PS->volume_fx < 0) {
			PS->volume_fx = 0;
		}
		Mix_Volume( -1, PS->volume_fx );
	}

	if (channel & VOLUME_MUSIC) {
		PS->volume_music -= VOLUME_STEPS;
		if (PS->volume_music < 0) {
			PS->volume_music = 0;
		}
		Mix_VolumeMusic( PS->volume_music );
	}
}

void hide_cursor( void )
{
#ifdef DISABLED_CODE
	int x, y;

	SDL_GetMouseState( &x, &y );
	SDL_WarpMouse( x, y );   //...what for is this needed anyways?
#endif
	SDL_ShowCursor( SDL_DISABLE );
}

void show_cursor( void )
{
#ifdef DISABLED_CODE
	int x, y;

	SDL_GetMouseState( &x, &y );
	SDL_WarpMouse( x, y );   //...what for is this needed anyways?
#endif
	SDL_ShowCursor( SDL_ENABLE );

}


/******************************************************************************
* HANDLE UI EVENTS
******************************************************************************/

void do_QUIT( program_state_t* PS ) {
	PS->run_mode = RM_EXIT;
	SDL_JoystickClose( PS->joystick );
}

void do_RESIZE( program_state_t* PS )
{
	int w = PS->window_width;
	int h = PS->window_height;

	printf(
		"Registering new window size: %dx%d\n",
		PS->window_width,
		PS->window_height
	);


#if START_IN_FULLSCREEN
	int screen_flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_FULLSCREEN;
#else
	int screen_flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE;
#endif

	PS->screen = SDL_SetVideoMode(
		w, h,
		SCREEN_BPP,
		screen_flags
	);
	if (PS->screen == NULL) {
		error_quit( "Resize: SDL_SetVideoMode() returned NULL" );
	}
}

void toggle_full_screen( program_state_t* PS )
{
#ifdef FULLSCREEN_WORKS_WITH_OPENGL
	uint32_t old_flags = PS->screen_flags;   // Save the current flags..
	                                         // ..in case toggling fails
	int w, h;

	if (PS->screen_flags & SDL_FULLSCREEN) {
		w = INITIAL_WINDOW_WIDTH;
		h = INITIAL_WINDOW_HEIGHT;
		PS->screen_flags &=~ SDL_FULLSCREEN;
	}
	else {
		const SDL_VideoInfo* info = SDL_GetVideoInfo();
		w = info->current_w;
		h = info->current_h;
		PS->screen_flags |= SDL_FULLSCREEN;
	}

	screen = SDL_SetVideoMode( w, h, SCREEN_BPP, PS->screen_flags );

	if (screen == NULL) {
		// If toggle FullScreen failed, then switch back
		screen = SDL_SetVideoMode(0, 0, 0, old_flags);
	}

	if (screen == NULL) {
		// If you can't switch back for some reason, then epic fail
		error_quit(
			"SDL_SetVideoMode() returned NULL"
			" when toggling fullscreen."
		);
	}
#endif
}

void test( program_state_t* PS, game_state_t* GS )   //...
{
	int i, j;
	formation_t* f = &(GS->formations[0]);

	for( i = 0 ; i < MAX_FORMATION_RANKS ; i++ ) {

		printf("Slot %d:", i);

		for( j = 0 ; j < NR_FILLFROM_RANKS ; j++ ) {
			printf( " %d", f->ranks[6].fillfrom_index[i] );
		}
		printf("\n");
	}


	enemy_t* e = GS->formations[0].ranks[0].occupied_by;

	e->ai_mode = FREE;

	if (sgn(e->velocity.x) == sgn(e->formation->velocity.x)) {
		e->velocity.x *= (-1);
	}

} // test


void set_test_status( game_state_t* GS )
{
	ship_t* s = &(GS->ship); 

	s->weapons[WEAPON_ROUNDSHOT].enabled = TRUE;
	GS->current_resource += 1000;

} // set_test_status



// http://content.gpwiki.org/index.php/OpenGL:Tutorials:Taking_a_Screenshot
SDL_Surface* flip_vert( SDL_Surface* sfc )
{

	SDL_Surface* result = SDL_CreateRGBSurface(
		sfc->flags,
		sfc->w,
		sfc->h,
		sfc->format->BytesPerPixel * 8,
		sfc->format->Rmask,
		sfc->format->Gmask,
		sfc->format->Bmask,
		sfc->format->Amask
	);
	if (result == NULL) return NULL;

	Uint8* pixels = (Uint8*) sfc->pixels;
	Uint8* rpixels = (Uint8*) result->pixels;

	Uint32 pitch = sfc->pitch;
	Uint32 pxlength = pitch*sfc->h;

	for(int line = 0; line < sfc->h; ++line) {
		Uint32 pos = line * pitch;
		memcpy( &rpixels[pos], &pixels[(pxlength-pos)-pitch], pitch );
	}

	return result;

} // flip_vert


/**
 * create_screenshot()
 * http://content.gpwiki.org/index.php/OpenGL:Tutorials:Taking_a_Screenshot
 */
void create_screenshot( program_state_t* PS, game_state_t* GS )
{
	SDL_Surface* surface = SDL_CreateRGBSurface(
		SDL_SWSURFACE, PS->window_width,
		PS->window_height,
		24,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0
	);

	if (surface == NULL) return;

	glReadPixels(
		0, 0,
		PS->window_width, PS->window_height,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		surface->pixels
	);

	SDL_Surface* flip = flip_vert( surface );
	if (flip == NULL) return;
	SDL_FreeSurface( surface );

	time_t rawtime;
	struct tm *ltime;

	time( &rawtime );
	ltime = localtime( &rawtime );

	char *screenshot_name;
	asprintf(
		&screenshot_name,
		"screenshot_%4d_%02d_%02d-%02d.%02d.%02d.bmp",
		ltime->tm_year + 1900,
		ltime->tm_mon,
		ltime->tm_mday,
		ltime->tm_hour,
		ltime->tm_min,
		ltime->tm_sec
	);

	SDL_SaveBMP( flip, screenshot_name );

	free( screenshot_name );
	SDL_FreeSurface( flip );

} // create_screenshot


void handle_keydown( program_state_t* PS, game_state_t* GS, int ksym )
{
	switch (ksym) {

	case SDLK_PRINT:        create_screenshot( PS, GS );    break;
	case SDLK_a:                                            // fall through
	case SDLK_j:                                            // fall through
	case SDLK_LEFT:         start_move( PS, GS, LEFT );     break;
	case SDLK_d:                                            // fall through
	case SDLK_l:                                            // fall through
	case SDLK_RIGHT:        start_move( PS, GS, RIGHT );    break;
	case SDLK_w:                                            // fall through
	case SDLK_i:                                            // fall through
	case SDLK_UP:           start_move( PS, GS, FORWARD );  break;
	case SDLK_s:                                            // fall through
	case SDLK_k:                                            // fall through
	case SDLK_DOWN:         start_move( PS, GS, BACK );     break;
#if DEBUG
	case SDLK_x:            remove_all_objects( GS );       break;
#endif
	case SDLK_m:            toggle_music();                 break;
	case SDLK_r:            reset_game( PS, GS );           break;
	case SDLK_t:            test( PS, GS );                 break;
	case SDLK_PLUS:         volume_up  ( PS, VOLUME_ALL );  break;
	case SDLK_MINUS:        volume_down( PS, VOLUME_ALL );  break;
	case SDLK_INSERT:                                       // fall through
	case SDLK_8:
	case SDLK_KP_PLUS:      volume_up  ( PS, VOLUME_FX );   break;
	case SDLK_DELETE:                                       // fall through
	case SDLK_7:
	case SDLK_KP_MINUS:     volume_down( PS, VOLUME_FX );   break;
	case SDLK_0:
	case SDLK_KP_MULTIPLY:  volume_up  ( PS, VOLUME_MUSIC );break;
	case SDLK_9:
	case SDLK_KP_DIVIDE:    volume_down( PS, VOLUME_MUSIC );break;
	case SDLK_COMMA:                                        // fall through
	case SDLK_RCTRL:                                        // fall through
	case SDLK_LCTRL:
		if (PS->run_mode == RM_RUNNING) {
			start_fire( PS, GS, WEAPON_LASER_1, FM_SINGLE );
		}
		break;
	case SDLK_PERIOD:                                       // fall through
	case SDLK_RSHIFT:                                       // fall through
	case SDLK_LSHIFT:                                       // fall through
	case SDLK_LSUPER:
		if (PS->run_mode == RM_RUNNING) {
			start_fire( PS, GS, WEAPON_LASER_2, FM_SINGLE );
		}
		break;
	case SDLK_RALT:                                         // fall through
	case SDLK_LALT:
		if (PS->run_mode == RM_RUNNING) {
			start_round_shot( PS, GS, FM_SINGLE );
		}
		break;
	case SDLK_RETURN:                                       // fall through
	case SDLK_SPACE:
		if (PS->run_mode & (RM_INTRO | RM_MAIN_MENU | RM_AFTER_LIFE)) {
			reset_game( PS, GS );
		}
		else {
			toggle_pause( PS );
		}
		break;

	case SDLK_ESCAPE:
#if DISABLED_CODE
		switch (PS->run_mode) {
			case RM_INTRO:
			case RM_PAUSE:
			case RM_MAIN_MENU:  do_QUIT( PS );
		}
#else
		if (PS->run_mode & (RM_INTRO | RM_PAUSE | RM_MAIN_MENU)) {
			do_QUIT( PS );
		}
#endif
	                                                        // fall through
	case SDLK_p:                                            // fall through
	case SDLK_PAUSE:  toggle_pause( PS );                   break;
	case SDLK_F3:     PS->debug = !PS->debug;               break;
	case SDLK_F4:     set_test_status( GS );                break;
	case SDLK_F11:    toggle_full_screen( PS );             break;
	case SDLK_F12:    error_quit("User abort [F12]");       break;

	} // case

} // handle_keydown


void handle_keyup( program_state_t* PS, game_state_t* GS, int ksym )
{
	switch (ksym) {
		case SDLK_a:                                    // fall through
		case SDLK_j:                                    // fall through
		case SDLK_LEFT:   stop_move( PS, GS, LEFT );    break;
		case SDLK_d:                                    // fall through
		case SDLK_l:                                    // fall through
		case SDLK_RIGHT:  stop_move( PS, GS, RIGHT );   break;
		case SDLK_w:                                    // fall through
		case SDLK_i:                                    // fall through
		case SDLK_UP:     stop_move( PS, GS, FORWARD );  break;
		case SDLK_s:                                    // fall through
		case SDLK_k:                                    // fall through
		case SDLK_DOWN:   stop_move( PS, GS, BACK );    break;
	}

} // handle_keyup


void handle_joyaxis( program_state_t* PS, game_state_t* GS, int axis, int value )
{
	int ksym;

	if ((axis == 1) || (axis == 2)) {
		if ((value < -3200 ) || (value > 3200)) {   // -32K ... +32K
			ksym = 0;
			if (axis == 1) ksym = (value < 0) ? SDLK_UP : SDLK_DOWN ;
			if (axis == 2) ksym = (value < 0) ? SDLK_LEFT : SDLK_RIGHT ;

			if (PS->joyaxis_key[ axis ] == 0) {
				handle_keydown( PS, GS, ksym );
				PS->joyaxis_key[ axis ] = ksym;
				printf("Pressing %d\n", ksym);
			}
		}
		else {
			ksym = PS->joyaxis_key[ axis ];
			if (ksym != 0) {
				handle_keyup( PS, GS, ksym );
				printf("Releasing %d\n", ksym);
			}
			PS->joyaxis_key[ axis ] = 0;
		}
	}

} // handle_joyaxis


void handle_joybuttondown( program_state_t* PS, game_state_t* GS, int button )
{
	switch (button) {  //... Autofire handled in process_event_queue() 
		case 0:  handle_keydown( PS, GS, SDLK_LCTRL );   break;
		case 1:  handle_keydown( PS, GS, SDLK_LSHIFT );  break;
		case 2:  handle_keydown( PS, GS, SDLK_LALT );    break;
		case 9:  handle_keydown( PS, GS, SDLK_SPACE );   break;
	}

} // handle_joybuttondown


void handle_joybuttonup( program_state_t* PS, game_state_t* GS, int button )
{
	switch (button) {
		case 0:  handle_keyup( PS, GS, SDLK_LCTRL );   break;
		case 1:  handle_keyup( PS, GS, SDLK_LSHIFT );  break;
		case 2:  handle_keyup( PS, GS, SDLK_LALT );    break;
		case 9:  handle_keyup( PS, GS, SDLK_SPACE );   break;
	}

} // handle_joybuttonup


void handle_joyhatmotion( program_state_t* PS, game_state_t* GS, int positions )
{
	if (positions != PS->joyhat) {
		// Register realease of button(s)
		if ((PS->joyhat & SDL_HAT_UP) && !(positions & SDL_HAT_UP)) {
			handle_keyup( PS, GS, SDLK_UP );
		}
		if ((PS->joyhat & SDL_HAT_RIGHT) && !(positions & SDL_HAT_RIGHT)) {
			handle_keyup( PS, GS, SDLK_RIGHT );
		}
		if ((PS->joyhat & SDL_HAT_DOWN) && !(positions & SDL_HAT_DOWN)) {
			handle_keyup( PS, GS, SDLK_DOWN );
		}
		if ((PS->joyhat & SDL_HAT_LEFT) && !(positions & SDL_HAT_LEFT)) {
			handle_keyup( PS, GS, SDLK_LEFT );
		}

		// Register new button(s)
		if (!(PS->joyhat & SDL_HAT_UP) && (positions & SDL_HAT_UP)) {
			handle_keydown( PS, GS, SDLK_UP );
		}
		if (!(PS->joyhat & SDL_HAT_RIGHT) && (positions & SDL_HAT_RIGHT)) {
			handle_keydown( PS, GS, SDLK_RIGHT );
		}
		if (!(PS->joyhat & SDL_HAT_DOWN) && (positions & SDL_HAT_DOWN)) {
			handle_keydown( PS, GS, SDLK_DOWN );
		}
		if (!(PS->joyhat & SDL_HAT_LEFT) && (positions & SDL_HAT_LEFT)) {
			handle_keydown( PS, GS, SDLK_LEFT );
		}

		PS->joyhat = positions;
	}

} // handle_joyhatmotion

void handle_mouse( program_state_t* PS )
{
	mouse_t* m = &(PS->mouse);
	microtime_t T = get_time();

	SDL_GetMouseState( &(m->x), &(m->y) );

	if ((abs(m->x - m->previous_x) > 3)
	&& (abs(m->y - m->previous_y) > 3))
	{
		show_cursor();
		m->visible_until_us = T + MOUSE_CURSOR_VISIBLE_US;
	}
	else {
		if (T > m->visible_until_us) {
			hide_cursor();
		}
	}

	m->previous_x = m->x;
	m->previous_y = m->y;
}

void process_event_queue( program_state_t* PS, game_state_t* GS )
{
	SDL_Event event;

	uint8_t *keystates = SDL_GetKeyState( NULL );

	handle_mouse( PS );   // Update internal mouse coordinates

	// Auto-Fire  //... Move this some better place. It is hard to add AF for the gamepad here.
	if (PS->run_mode == RM_RUNNING) {

		if( keystates[SDLK_LCTRL]
		||  keystates[SDLK_RCTRL]
		||  keystates[SDLK_COMMA]
		) {
			continue_fire( PS, GS, WEAPON_LASER_1 );
		}

		if( keystates[SDLK_LSHIFT]
		||  keystates[SDLK_LSUPER]
		||  keystates[SDLK_RSHIFT]
		||  keystates[SDLK_RALT]
		) {
			continue_fire( PS, GS, WEAPON_LASER_2 );
		}

		if( keystates[SDLK_LALT]
		||  keystates[SDLK_RALT]
		) {
			continue_fire( PS, GS, WEAPON_ROUNDSHOT );
		}
	}

	// Process event queue
	while (SDL_PollEvent(&event)) {

		switch (event.type) {
			case SDL_VIDEORESIZE:
				PS->window_width  = event.resize.w;
				PS->window_height = event.resize.h;
				do_RESIZE( PS );
				break;

			case SDL_ACTIVEEVENT:
				if( (event.active.gain != 1)
				&&  (PS->run_mode == RM_RUNNING)
				) {
					// Pause game, when the window..
					// ..looses input focus
					PS->run_mode = RM_PAUSE;
				}
				break;

			case SDL_KEYDOWN:
				handle_keydown( PS, GS, event.key.keysym.sym );
				break;

			case SDL_KEYUP:
				handle_keyup( PS, GS, event.key.keysym.sym );
				break;

			case SDL_JOYBUTTONDOWN:
				handle_joybuttondown(
					PS, GS,
					event.jbutton.button
				);
				break;

			case SDL_JOYBUTTONUP:
				handle_joybuttonup(
					PS, GS,
					event.jbutton.button
				);
				break;

			case SDL_JOYHATMOTION:
				handle_joyhatmotion(
					PS, GS,
					event.jhat.value
				);
				break;

			case SDL_JOYAXISMOTION:
				handle_joyaxis(
					PS, GS,
					event.jaxis.axis,
					event.jaxis.value
				);
				break;

#ifdef DISABLED_CODE
			case SDL_MOUSEMOTION:
				PS->mouse.position_x = event.motion.x;
				PS->mouse.position_y = event.motion.y;
				break;
#endif

			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
				case SDL_BUTTON_LEFT:
					PS->mouse.button.left = TRUE;
					break;
				case SDL_BUTTON_RIGHT:
					PS->mouse.button.right = TRUE;
					break;
				case SDL_BUTTON_WHEELUP:
					break;
				case SDL_BUTTON_WHEELDOWN:
					break;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				switch (event.button.button) {
				case SDL_BUTTON_LEFT:
					PS->mouse.button.left = FALSE;
					break;
				case SDL_BUTTON_RIGHT:
					PS->mouse.button.right = FALSE;
					break;
				}
				break;

			case SDL_QUIT:
				do_QUIT( PS );
				break;
		}
	}
}


//EOF
