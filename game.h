//game.h
/******************************************************************************
* GAME LOGIC
*******************************************************************************
* Configure details of the game here. The struct  game_state  holds all
* information about the current game, like positions, score, aso.
* The module provides functions for game control (resetting the game, saving
* highscore to disk, controlled random) and common helper functions.
******************************************************************************/
#ifndef GAME_LOGIC_H_PARSED
#define GAME_LOGIC_H_PARSED


#include <SDL/SDL_mixer.h>

#include "main.h"
#include "presets.h"


/******************************************************************************
* GAME STATE
******************************************************************************/

/* "SUPER" STRUCT  game_state_t
 * Consists of structs describing every aspect of the game. The instance of the
 * variable holding the current game state is defined as a global in  main.c .
 */

typedef enum ai_modes_e {
	FREE, FORMATION, RANK_TRANSIT, FOLLOW, CRASH_INTO, ORBIT
} ai_modes_t;

typedef struct thruster_state_s	thruster_state_t;	// Ship (player)
typedef struct weapon_s		weapon_t;
typedef struct ship_s		ship_t;

typedef struct formation_rank_s	formation_rank_t;	// Enemy
typedef struct formation_s	formation_t;
typedef struct enemy_s		enemy_t;

typedef struct laser_beam_s	laser_beam_t;		// Various objects
typedef struct explosion_s	explosion_t;
typedef struct black_hole_s	black_hole_t;
typedef struct bonus_bubble_s	bonus_bubble_t;

typedef struct camera_s		camera_t;		// UI
typedef struct sounds_s		sounds_t;

typedef struct score_info_s	score_info_t;		// Game
typedef struct game_state_s	game_state_t;


// STRUCTS ////////////////////////////////////////////////////////////////////

struct thruster_state_s {
	microtime_t forward_until_us;
	microtime_t pay_after_us;

	bool_t	left, right;
	bool_t	forward, back;
};

struct weapon_s {
	bool_t enabled;			// Must be achieved in-game to enable
	microtime_t blink_start_us;	// HUD: When the blinking started
	microtime_t blink_until_us;	// HUD: When to stop blinking
	microtime_t fire_next_beam_us;	// When the next auto fire shot
					// will be fired (used for the
					// initial delay before auto-
					// fire starts)
	int auto_fire_count ;		// Used to limit burst length
};

struct ship_s {
	microtime_t indicate_hit_until_us;

	vector_t position;
	vector_t velocity;
	real_t speed;

	thruster_state_t thruster_state;
	weapon_t weapons[NR_WEAPONS];

	real_t distance_to_black_hole;

	real_t camera_speed_pitch;	// Move camera according to speed..
	real_t speed_target_pitch;	// ..but do the change smoothly
};


// ENEMY //////////////////////////////////////////////////////////////////////

struct formation_rank_s {
	enemy_t* occupied_by;
	vector_t position;		// Relative to formation.position

	coordinate_t coordinate;
	int fillfrom_index[NR_FILLFROM_RANKS];	//... int --> formation_rank_t*
};

struct formation_s {
	vector_t position;
	vector_t velocity;

	formation_rank_t ranks[MAX_FORMATION_RANKS];
	int nr_ranks;
};

struct enemy_s {
	bool_t active;
	microtime_t indicate_hit_until_us;	// Blink
	microtime_t shoot_wait_until_us;	// Hold fire until time

	vector_t position;
	vector_t velocity;
	color_t color;

	int tier;
	int agressiveness;
	int hit_points;
	int score;

	ai_modes_t ai_mode;			// How the enemy is controlled

	formation_t* formation;			// NULL if freely moving
	formation_rank_t* current_rank;		// Where the enemy "parks"
	formation_rank_t* target_rank;		// When changing position..
	microtime_t rank_change_start_us;	// ..within the formation
};


// VARIOUS OBJECTS ////////////////////////////////////////////////////////////

struct laser_beam_s {
	bool_t active;

	vector_t position;
	vector_t velocity;

	int owner;		// Less than zero: Player weapon
	real_t decay_beyond_y;	// Built-in self-destruct ("firing range")
	real_t speed_bonus;	// Fired at slow speed reduces Resource gain
};

struct explosion_s {
	bool_t active;
	microtime_t start_time_us;

	vector_t position;
};

struct black_hole_s {
	vector_t position;
	vector_t velocity;

	int random_seed;

	vector_t camera_rotation;
	vector_t ship_translation;
};

struct bonus_bubble_s {
	bool_t active;
	microtime_t start_time_us;

	vector_t position;
	color_t color;

	int tier;		// Which weapon this bubble may unlock.
	int resource;		// How much Resource the player may gather
};


// UI /////////////////////////////////////////////////////////////////////////

struct camera_s {
	vector_t position;

	real_t distance;
	real_t angle;
};

struct sounds_s {
	Mix_Chunk* laser;
	Mix_Chunk* hit;
	Mix_Chunk* punch;
	Mix_Chunk* blast;
	Mix_Chunk* denied;
	Mix_Chunk* alarm;
	Mix_Chunk* blub;

	Mix_Music* music;
};


// GAME ///////////////////////////////////////////////////////////////////////

struct score_info_s {
	int current;		// Collected score by killing enemies
	int high_score;		// Overall high score

	int distance;		// Total score values calculated in..
	int speed;		// .. calulate_total_score()
	int best_resource;
	int hit_ratio;
};

struct game_state_s {
	int current_level;

	ship_t		ship;
	black_hole_t	black_hole;

	formation_t	formations	[MAX_FORMATIONS];
	enemy_t		enemies		[MAX_ENEMIES];
	laser_beam_t	laser_beams	[MAX_LASER_BEAMS];
	explosion_t	explosions	[MAX_EXPLOSIONS];
	bonus_bubble_t	bonus_bubbles	[MAX_BONUS_BUBBLES];

	int		nr_active_formations;

	int		nr_active_lasers;
	int		nr_active_explosions;

	int		nr_active_enemies_total;
	int		nr_active_enemies[NR_TIERS];

	int		nr_active_bonus_bubbles_total;
	int		nr_active_bonus_bubbles[NR_TIERS];

	camera_t	camera;
	sounds_t 	sounds;

	int shots_fired;	// How many beams the player started so far
	int shots_en_route;	// How many of those beams are still active
	int shots_missed;	// Beams decayed before they hit anything

	int current_resource;	// Current Resource amount
	int best_resource;	// Highest Resource recorded in this round

	score_info_t	score;
	microtime_t	blink_highscore_until_us;

	// Automatic recharging
	microtime_t next_recharge_after_us;	// At least travel for a while
	real_t next_recharge_beyond_y;	// At least travel beyond this y pos.

	real_t add_enemy_beyond_y;	// Penalty extra enemies, when the..
	int nr_warp_enemies;		// ..player doesn't kill within one
					// FIELD_HEIGHT (one "warp around")

	// Prototypal
	int formation_random_seed;

}; // game_state_t


/******************************************************************************
* PROTOTYPES
******************************************************************************/

void blink_weapon_hud(
	program_state_t* PS,
	weapon_t* w,
	microtime_t blink_duration
);
void disable_best_weapon( program_state_t* PS, game_state_t* GS );

real_t enemy_size( game_state_t* GS, enemy_t* enemy );
real_t bubble_size( bonus_bubble_t* b );
int get_tier_color( int tier, real_t* R, real_t* G, real_t* B );
real_t get_tier_hitpoints( int tier );

real_t level_length( game_state_t* GS );
void normalize_position_y( game_state_t* GS, vector_t* position );

int calculate_hit_points( real_t resource );
real_t calculate_hit_ratio( game_state_t* GS );

int rand_int( int* random_seed );

int calculate_total_score( program_state_t* PS, game_state_t* GS );
void load_highscore( game_state_t* GS );
void save_highscore( program_state_t* PS, game_state_t* GS );

void reset_game( program_state_t* PS, game_state_t* GS );
void toggle_pause( program_state_t* PS );


#endif
//EOF
