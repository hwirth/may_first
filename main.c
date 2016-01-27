//main.c
/******************************************************************************
* MAY FIRST - A game inspired by http://en.wikipedia.org/wiki/Juno_First
*******************************************************************************
* The main control loop handles control of frame rate and calls various
* functions that make up the game, like input handling, screen updates, etc.
* Some basic mathematical functions and types are kept here as well.
******************************************************************************/

#include <stdarg.h>
#include <sys/time.h>     // gettimeofday()

#include "main.h"         // Flow control, program state, run modes
#include "draw_frame.h"   // Renders the current game state to screen
#include "game.h"         // Game related settings, game state control
#include "world.h"        // Simulation, advances game state step wise
#include "ui.h"           // Window settings, process_event_queue()


// GLOBAL VARIABLES ///////////////////////////////////////////////////////////

program_state_t program_state = {
	.run_mode        = RM_INIT,
	.window_width    = INITIAL_WINDOW_WIDTH,
	.window_height   = INITIAL_WINDOW_HEIGHT,
	.next_fps_sample = 0,
	.frame_times     = {0},   // See:
	// http://stackoverflow.com/questions/5636070/zero-an-array-in-c-code
	.initial_TAS     = -1,
	.debug           = DEBUG,

#if START_IN_FULLSCREEN
	.screen_flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_FULLSCREEN,
#else
	.screen_flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE,
#endif
};

game_state_t game_state;


// COMMON HELPERS /////////////////////////////////////////////////////////////

void debugf( bool_t do_print, char* format_string, ... )
{
#if DEBUG
	if (do_print) {
		va_list argp;

		va_start( argp, format_string );
		vprintf( format_string, argp );
		va_end(argp);
	}
#endif
}

/* error_quit()
 * When an error occurs, this function is used to terminate the program with
 * a useful error message.
 */
void error_quit( char* error_message )
{
	fprintf( stderr, "Error: %s\n", error_message );
	exit( -1 );
}

/* get_time()
 * Returns time in microseconds.
 */
microtime_t get_time( void )
{
	struct timeval t;
	gettimeofday( &t, NULL );
	return ((microtime_t)t.tv_sec * 1000000) + t.tv_usec;
}


// MATH HELPERS ///////////////////////////////////////////////////////////////

vector_t vector( real_t x, real_t y, real_t z )
{
	vector_t ret;

	ret.x = x;
	ret.y = y;
	ret.z = z;

	return ret;
}

void round_vector( vector_t* v )
{
	v->x = round( v->x );
	v->y = round( v->y );
	v->z = round( v->z );
}

vector_t add_vector( vector_t base, vector_t offset )
{
	vector_t result;

	result.x = base.x + offset.x;
	result.y = base.y + offset.y;
	result.z = base.z + offset.z;

	return result;
}

vector_t subtract_vector( vector_t base, vector_t offset )
{
	vector_t result;

	result.x = base.x - offset.x;
	result.y = base.y - offset.y;
	result.z = base.z - offset.z;

	return result;
}

real_t vector_length( vector_t v )
{
	return sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
}

vector_t multiply_vector_scalar( vector_t v, real_t scalar )
{
	v.x *= scalar;
	v.y *= scalar;
	v.z *= scalar;

	return v;
}

vector_t unity_vector( vector_t v )
{
	real_t length = vector_length(v);

	if (length == 0) {
		fprintf( stderr, "unity_vector(): length == 0\n" );
		return multiply_vector_scalar( v, 0 );
	}
	else {
		return multiply_vector_scalar( v, 1/length );
	}
}

int sgn( int r )
{
	return (r > 0) - (r < 0);
}

real_t fsgn( real_t r )
{
	return (r > 0) - (r < 0);
}

int min( int a, int b )
{
	return (a < b) ? a : b ;
}

int max( int a, int b )
{
	return (a > b) ? a : b ;
}


// MAIN PROGRAM ///////////////////////////////////////////////////////////////

/* save_frame_time()
 * Stores a record of measured frame calculation time into an array used for
 * finding the average frame rate over the last  NR_FPS_SAMPLES  samples.
 */
void save_frame_time( program_state_t* PS )
{
	++PS->next_fps_sample;
	PS->next_fps_sample %= NR_FPS_SAMPLES;
	PS->frame_times[ PS->next_fps_sample ] = PS->frame_time_us;
}

/* calculate_frames_per_second()
 * Returns the average for all  NR_FPS_SAMPLES  samples
 */
real_t average_frame_time( program_state_t* PS )
{
	int i;
	int active_samples = 0;
	microtime_t Tsum = 0;

	for( i = 0 ; i < NR_FPS_SAMPLES ; i++ ) {
		if (PS->frame_times[i] > 0) {
			++active_samples;
			Tsum += PS->frame_times[i];
		}
	}

	return Tsum / (real_t)active_samples;
}

/* main()
 * Program entry point. Initializes everything and runs the main loop.
 * Times are calculated and the frame rate is controlled here.
 */
int main( int argc, char* argv[] )
{
	program_state_t* PS  = &program_state;
	game_state_t*    GS  = &game_state;
	sounds_t*      	 snd = &(GS->sounds);

	PS->program_start_us = get_time();

	init_sdl( PS, GS );     // Create window, load sounds
	init_sound( PS, GS );   // Load music, sounds, set volume
	init_font( PS );        // Load font
	init_opengl( PS );      // Load textures

	play_music( snd->music );

	load_highscore( GS );

	PS->current_time_us    =
	PS->frame_start_us     = get_time();

	// Make the following look nice in debug info
	PS->game_start_us      =
	PS->pause_since_us     = PS->program_start_us - 1;

	PS->tick_fraction_s    = -1;

	PS->highest_frame_time = 0;
	PS->lowest_frame_time  = 99999999;

	PS->run_mode = RM_INTRO;

	while (PS->run_mode != RM_EXIT) {

		PS->current_time_us = get_time();

		// Calculate how much time we need to waste for target FPS
		PS->delay_until_us
			= PS->current_time_us
			+ (1000000 / TARGET_FPS)
		;


		// Keep track of game time (handling pause)
		if ( !(PS->run_mode & (RM_PAUSE | RM_MAIN_MENU)) ) {
			PS->game_time_us
				= PS->current_time_us
				- PS->game_start_us
			;

		}

		// Show intro after main menu idles for a while
		if (PS->run_mode == RM_MAIN_MENU) {
			if (PS->main_menu_since_us
				+ MAIN_MENU_INTRO_SWITCH_TIME < get_time()) {
				PS->run_mode = RM_INTRO;
			}
		}

		// Handle user input
		process_event_queue( PS, GS );

		// Let the universe live for a short moment
		if (PS->run_mode & (RM_RUNNING | RM_AFTER_LIFE)) {
			advance_simulation( PS, GS );
		}

		// Display the current world status as game scene
		draw_frame( PS, GS );

#if LIMIT_FPS
		// Waste time until the next frame needs to be drawn
		while (get_time() < PS->delay_until_us) {
	#if LIMIT_FPS_USING_SDL_DELAY
			SDL_Delay( 1 );
	#endif
		}
#endif
		// Keep track of how many FPS we actually achieved
		PS->frame_time_us   = get_time() - PS->frame_start_us;

		PS->highest_frame_time = max(
			PS->highest_frame_time,
			PS->frame_time_us
		);
		PS->lowest_frame_time = min(
			PS->lowest_frame_time,
			PS->frame_time_us
		);

		PS->frame_start_us  = get_time();
		PS->tick_fraction_s = (real_t)PS->frame_time_us / 1000000.0;

		save_frame_time( PS );   // For average FPS
		PS->average_frame_time = average_frame_time( PS );
	}

	save_highscore( PS, GS );

	glDeleteTextures( 1, &(PS->textures.background) );
	glDeleteTextures( 1, &(PS->textures.digits) );

	Mix_FreeChunk( snd->laser );
	Mix_FreeChunk( snd->hit );
	Mix_FreeChunk( snd->punch );
	Mix_FreeChunk( snd->blast );
	Mix_FreeChunk( snd->denied );
	Mix_FreeChunk( snd->alarm );
	Mix_FreeChunk( snd->blub );

	Mix_FreeChunk( snd->computer_autofire );
	Mix_FreeChunk( snd->computer_doubleshot );
	Mix_FreeChunk( snd->computer_roundshot );
	Mix_FreeChunk( snd->computer_danger );
	Mix_FreeChunk( snd->computer_weaponlost );

	Mix_FreeMusic( snd->music );

	SDL_Quit();   // Will (allegedly) also free  screen .

	return 0;
}


//EOF
