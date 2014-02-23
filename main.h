//main.h
#define PROGRAM_NAME	"MAY FIRST"
#define PROGRAM_VERSION	"alpha0.2.4"
/******************************************************************************
* MAY FIRST - A game inspired by http://en.wikipedia.org/wiki/Juno_First
*******************************************************************************
* You can change a few settings related to main flow control (unrelated to the
* game itself) here.
* The struct  program_state  holds all information about general things like
* times (currently including game related times, which are provided to the game
* routines as a "service" of the main engine), window size, aso.
******************************************************************************/
#ifndef MAIN_H_PARSED
#define MAIN_H_PARSED


#include <stdint.h>		//...? uint32_t for times
#include <math.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_opengl.h>


// OPTIONS ////////////////////////////////////////////////////////////////////
// More settings are found in  game.h  and  ui.h .

/* //...Unbearable stuttering with lower frame rate, but the
 * frame rate appears to be capped to 60 frames/s anyways.
 * Another machine seems to cap the frame rate at 80 frames/s.
 * (Not anymiore, now it is 60-65 FPS
 * See also: http://www.opengl.org/wiki/Swap_Interval
 * Try  export vblank_mode=0
 * Try
 *   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
 *   SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
 *   (See  draw_frame.c:init_opengl() )
 */
#define TARGET_FPS			60	//...Check strange 60 FPS limit
#define LIMIT_FPS			FALSE	//...tearing!?
#define LIMIT_FPS_USING_SDL_DELAY	TRUE	// Wait using SDL_Delay() in..
						// ..order to save CPU power

#define DEBUG				TRUE	// Enable debug output in..
						// ..game [F3] and to stdout
// Effective only when  DEBUG == TRUE :
#define DEBUG_TIMES			TRUE	// [F3]-screen details
#define DEBUG_SHIP			TRUE
#define DEBUG_MEMORY			TRUE	// !(__APPLE__ && __MACH__)
#define DEBUG_GAME_STATISTICS		TRUE

#define DEBUG_FORMATION_CREATION	FALSE	// Debug to stdout
#define DEBUG_FORMATION_RANKS		FALSE

#define CHEAT_MODE			FALSE	// Very easy gameplay
#define FREEZE_ENEMIES			FALSE	// Enemies do not move
#define FREEZE_ENEMY_WEAPONS		FALSE	// Enemies do not fire
#define TEST_LEVELS			FALSE


// DEFINES ////////////////////////////////////////////////////////////////////

#define NR_FPS_SAMPLES	100		// The last 100 times will be kept to..
					// ..get a steady average FPS value
#define RM_INIT		1
#define RM_INTRO	2
#define RM_EXIT		4
#define RM_MAIN_MENU	8
#define RM_RUNNING	16
#define RM_AFTER_LIFE	32
#define RM_PAUSE	64

#define DEG_TO_RAD	((2*M_PI) / 360.0)
#define RAD_TO_DEG	(360.0 / (2*M_PI))


// STRUCT TYPEDEFS ////////////////////////////////////////////////////////////

typedef struct coordinate_s	coordinate_t;
typedef struct vector_s		vector_t;
typedef struct color_s		color_t;

typedef struct textures_s	textures_t;

typedef struct button_s		button_t;
typedef struct mouse_s		mouse_t;

typedef struct program_state_s	program_state_t;


// COMMON TYPES ///////////////////////////////////////////////////////////////

typedef int	bool_t;
#define TRUE	1
#define FALSE	0

typedef unsigned long long microtime_t;

typedef GLfloat	real_t;		//... double?

struct coordinate_s	{ int x, y; };
struct vector_s		{ real_t x, y, z; };
struct color_s		{ real_t R, G, B; };


// OpenGL /////////////////////////////////////////////////////////////////////

struct textures_s {
	GLuint background;
	GLuint digits;
};


// Mouse //////////////////////////////////////////////////////////////////////

struct button_s {
	bool_t left;
	bool_t right;
};

struct mouse_s {
	int x, previous_x;
	int y, previous_y;
	button_t button;
	microtime_t visible_until_us;
};


// PROGRAM STATE //////////////////////////////////////////////////////////////

struct program_state_s {
	int run_mode;
	bool_t debug;				// Show information, when TRUE

	int next_fps_sample;			// Index to  frame_times[]
	microtime_t frame_times[NR_FPS_SAMPLES];
	microtime_t average_frame_time;
	microtime_t lowest_frame_time;
	microtime_t highest_frame_time;

	// Main control
	microtime_t current_time_us;		// Is set at begin of main loop
	microtime_t frame_start_us;		// Used for calculating..
	microtime_t frame_time_us;		// ..tick_fraction_s
	microtime_t delay_until_us;		// SDL_Sleep against 100% CPU

	// Game control
	microtime_t program_start_us;		// Retreived early, only once
	microtime_t game_start_us;		// Current game started
	microtime_t game_time_us;		// Since game start
	microtime_t after_life_start_us;	// Continue explosion animation
	microtime_t pause_since_us;		// Updating game_start_us
	microtime_t main_menu_since_us;		// Show intro after a while
	microtime_t next_second_us;		// Trigger action every second
	real_t tick_fraction_s;

	// OpenGL
	textures_t	textures;		// Struct holding all textures

	// User interface
	int window_width;
	int window_height;
	uint32_t screen_flags;

	SDL_Surface*	screen;
	TTF_Font*	font;

	mouse_t		mouse;

	int font_size, line_height;
	int volume_fx, volume_music;

	// Debug
	uint32_t initial_TAS;

}; // program_state_t


// PROTOTYPES /////////////////////////////////////////////////////////////////

void debugf( bool_t do_print, char* format_string, ... );

microtime_t get_time( void );

void error_quit( char* error_message );

vector_t vector( real_t x, real_t y, real_t z );
void round_vector( vector_t* v );
vector_t add_vector( vector_t v, vector_t offset );
vector_t subtract_vector( vector_t v, vector_t offset );
real_t vector_length( vector_t v );
vector_t multiply_vector_scalar( vector_t v, real_t scalar );
vector_t unity_vector( vector_t v );

real_t fsgn( real_t r );
int sgn( int r );
int min( int a, int b );
int max( int a, int b );


#endif
//EOF
