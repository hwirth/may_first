//presets.h
/******************************************************************************************************************120*
* GAME PRESETS
***********************************************************************************************************************
* Configure details of the game here.
**********************************************************************************************************************/
#ifndef PRESETS_H_PARSED
#define PRESETS_H_PARSED


// OPTIONS ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DRAW_BLACK_HOLES		TRUE	//TRUE Visually show the holes on the grid
#define BLACK_HOLES_SUCK		TRUE	//TRUE Disabled: Player can fly over the holes
#define BLACK_HOLES_DARKEN_SCREEN	TRUE	//TRUE Dim screen while approaching center of b.h.
#define BLACK_HOLES_OPEN_ON_APPROACH	TRUE	//TRUE The nearer the shop, the larger the hole

#define LIMIT_FORWARD_SPEEDING		TRUE	//FALSE Disable this to speed endlessly
#define FIRE_DIAGONALLY			FALSE	//FALSE Fire in the direction, the ship is moving (horizontally)

#define LINE_MARKERS			TRUE	//TRUE Indicate every 100 units
#define LINE_LABELS			TRUE	//TRUE Show "milestone" numbers
						// ..labels forward
#define SHOW_BACKGROUND			TRUE	//TRUE BACKGROUND_FILENAME

#define COLORED_EXPLOSIONS		FALSE
#define SPATIAL_EXPLOSIONS		FALSE	//FALSE Somehow broken, looks like pixel noise
#define ENEMIES_ROTATE_INDIVIDUALLY	FALSE	//FALSE Looks better if they dance synchronously

#define RANDOM_LEVELS			FALSE	//...not.used Randomized level generation

#define CURVED_FORMATIONS		FALSE	// FALSE means rectangular arrangement

#define ENEMIES_CHANGE_DIRECTION_ON_HIT	FALSE	// This feature should be used for a specific (future) AI mode

// DEFINES ////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HIGHSCORE_FILENAME		"./highscore.dat"
#define DIGITS_FILENAME			"./media_files/digits.png"
#define BACKGROUND_FILENAME		"./media_files/stars_background.jpg"
#define BACKGROUND_INTENSITY		96	// 0..255 (transparent --> opaque)

// http://lazyfoo.net/SDL_tutorials/lesson11/

#define COMPUTER_AUTOFIRE_WAV		"./media_files/computer_autofire.wav"
#define COMPUTER_DOUBLESHOT_WAV		"./media_files/computer_doubleshot.wav"
#define COMPUTER_ROUNDSHOT_WAV		"./media_files/computer_roundshot.wav"
#define COMPUTER_DANGER_WAV		"./media_files/computer_danger.wav"
#define COMPUTER_WEAPONLOST_WAV		"./media_files/computer_weaponlost.wav"

#define LASER_WAV			"./media_files/laser.wav"
#define HIT_WAV				"./media_files/hit.wav"
#define PUNCH_WAV			"./media_files/punch.wav"
#define BLAST_WAV			"./media_files/blast.wav"
#define DENIED_WAV			"./media_files/denied.wav"
#define ALARM_WAV			"./media_files/alarm.wav"
#define BLUB_WAV			"./media_files/blub.wav"

#define GAME_MUSIC			"./media_files/neu4_run.ogg"


#define MAIN_MENU_INTRO_SWITCH_TIME	(1000000 * 930)	// After death, score is shown..
							// ..for this amount of time.

#define FIELD_WIDTH			200.0		// Size of the field
#define FIELD_HEIGHT			1000.0		//850.0

#define FIELD_MIN_X			(FIELD_WIDTH / -2.0)
#define FIELD_MAX_X			(FIELD_WIDTH / +2.0)
#define FIELD_MIN_Y			(FIELD_HEIGHT / -10.0)	// Range of the field, bottom (behind the player)
#define FIELD_MAX_Y			(FIELD_HEIGHT / +2.0)	// forward

#define FIELD_STEP_X			5.0		// Distance between dots
#define FIELD_STEP_Y			5.0

#define LABEL_COLOR			0.0, 0.6, 0.8
#define LABEL_OFFSET_X			0.2		// Font center position tweak
#define LABEL_OFFSET_Y			0.0		// Positive values move the..
#define LABEL_QUAD_SIZE			0.8
#define LABEL_QUAD_WIDTH		(5.0*LABEL_QUAD_SIZE)
#define LABEL_QUAD_HEIGHT		(8.0*LABEL_QUAD_SIZE)

#define SHIP_SIZE			2.0		// Actual size is hard coded, SHIP_SIZE is for collision detection
#define SHIP_WIDTH			4		// Dimensions of the ship..
#define SHIP_HEIGHT			2		// ..regarding collision..
#define SHIP_LENGTH			4		// ..detection
#define SHIP_MIN_X			(FIELD_WIDTH/-2 + SHIP_WIDTH/2)	// Movement range
#define SHIP_MAX_X			(FIELD_WIDTH/2 - SHIP_WIDTH/2)

#define SPEED_X				75.0		// Units per second
#define SPEED_Y				100.0		// Units per second

#define FORWARD_SPEED_FACTOR    	2.5
#define BACK_SPEED_FACTOR		-0.35		// No parentheses to prevent rounding
#define FORWARD_THRUSTING_MAX_TIME	(1000000*5)	// How long, until thrusting will be interrupted
#define FORWARD_THRUSTING_PAY_TIME	(1000000/5)	// How long, until Resource will be reduced again
#define FORWARD_RESOURCE_COST		1		//1 Resource per FORWARD_THRUSTING_PAY_TIME

#define INITIAL_CAMERA_ANGLE		-65	//-58.0
#define INITIAL_CAMERA_DISTANCE		110	//120.0
#define CAMERA_OFFSET_X			20.0		// How much the camera should move according to x-position
#define CAMERA_OFFSET_Y			-30.0		//-30 Constant translation of the camera
#define CAMERA_ROTATION_ROLL_FACTOR	-5.0 		//-5  How far the cammera should pitch
#define CAMERA_ROTATION_CHANGE_SPEED	2.0		//5 How fast the camera turns when speed changes (40-60)
#define CAMERA_DISTANCE_SPEED_FACTOR	20.0		// How much subtracted when  camera_speed_pitch == 1 .
#define CAMERA_PITCH_OFFSET_Y		3.0

#define GRID_CELL_SIZE			5
#define GRID_DELTA_Z			-1		// How far the grid is to the ship
#define GRID_DEPTH_FACTOR		0.85		// Tweak to pull the darkness towards the ship
#define GRID_CHEAT_OFFSET_Y		-50.0		// Somehow, the formula is not good

#define BLACK_HOLE_RADIUS_SHIP		50.0		// Radius for ship offset z
#define BLACK_HOLE_RADIUS_DARKNESS	45		//35 Radius for darkness
#define BLACK_HOLE_RADIUS_RESOURCE	35
#define BLACK_HOLE_SPEED_X		0.5		// The black hole is moving slowly at this speed
#define BLACK_HOLE_SPEED_Y		0.5
#define BLACK_HOLE_CHEAT_OFFSET_Y	-50.0		// Used when calculating distance to ship


#define NR_WEAPONS			4

#define HUD_BLINK_SPEED			300000
#define ACHIEVEMENT_BLINK_DURATION	1500000		// Blink in HUD when weapon activated
#define ACHIEVEMENT_LOST_BLINK_DURATION	ACHIEVEMENT_BLINK_DURATION
#define DENIED_BLINK_DURATION		HUD_BLINK_SPEED

#define WEAPON_LASER_1			0		// Indices to the array
#define WEAPON_LASER_2			1
#define WEAPON_ROUNDSHOT		2
#define WEAPON_AUTOFIRE			3

#define COLOR_TEXT_HI			0x88FFCC
#define COLOR_TEXT_LO			0x44CCAA
#define COLOR_ENABLED_HI		0x00CC00
#define COLOR_ENABLED_LO		0x006600
#define COLOR_DISABLED_HI		0xCC0000
#define COLOR_DISABLED_LO		0x992200
#define COLOR_BLINK_HI			0xFFFFFF
#define COLOR_BLINK_LO			0x000000

#define DUAL_FIRE_DISTANCE		2.5		// Starting position x-offset for dual LASER

#define MAX_LASER_BEAMS			1000		// Max. amount of simultaneously active (visible) laser beams
#define MAX_ENEMIES			100		// Max.
#define MAX_EXPLOSIONS			MAX_ENEMIES
#define MAX_BONUS_BUBBLES		MAX_ENEMIES
#define MOVING_ANGLE_OFFSET		75.0		// Make the shot move to the side, when fired while moving

#define LASER_SPEED_PLAYER		300.0		// Speed in units per second
#define LASER_SPEED_ENEMY		200.0		// Speed in units per second
#define LASER_SPEED_MOTHERSHIP		300.0
#define LASER_LENGTH_FACTOR		5.0		// Length of a LASER beam

#define INITIAL_RESOURCE		99.0		//100.0 Start with some Resource reserve
#define LASER_HIT_PENALTY_MIN		50.0		//50 Score subtracted, when player is hit
#define RECHARGE_DISTANCE		100.0		//100.0 Distance must be traveled to regain Resource
#define RECHARGE_TIME			(1000000)	//3.0   Time must have passed to regain Resource
#define RECHARGE_RESOURCE_AMOUNT	1.0		//1.0   Permanently gain a little Resource
#define HIT_INDICATION_TIME_US		75000		// Flash the object white to indicate a non-lethal hit
#define AFTER_LIFE_DURATION_US		500000		// Extra run-level to let animations finish after death

#define FIRING_COST_LASER_1		1
#define FIRING_COST_LASER_2		4
#define FIRING_COST_ROUND_SHOT		20		//100

#define AUTO_FIRE_MALUS			2		// Additional firing cost
#define AUTO_FIRE_DELAY			222000		// Time to wait before auto fire starts
#define AUTO_FIRE_INTERVAL		75000		// Time between single shots
#define AUTO_FIRE_MAX_SHOTS		5		// How many shots in one burst (including the initial shot)
#define FM_SINGLE			FALSE		// Firing modes
#define FM_AUTO				TRUE

#define OWNER_PLAYER			(-1)		// Keep track of who fired which laser beam
#define OWNER_PLAYER_NO_RESPAWN		(-2)		// Beams that should not trigger enemy respawn

#define NEXT_WAVE_NOTICE_DURATION	(4*1000000)
#define NEXT_WAVE_PAUSE_OFFSET		0.1		// Factor of FIELD_HEIGHT of moving the first formations additionally further behind

#define NR_TIERS			4		// Kinds of enemies
#define TIER_1				0
#define TIER_2				1
#define TIER_3				2
#define TIER_4				3
#define MOTHERSHIP_TIER			TIER_4

#define MAX_FORMATIONS			64
#define MAX_FORMATION_SIZE		8
#define MAX_FORMATION_RANKS		(MAX_FORMATION_SIZE * MAX_FORMATION_SIZE)
#define NO_FORMATION			NULL
#define NR_FILLFROM_RANKS		5
#define RANK_TRANSIT_TIME		1000000

#define AI_MAX_SHOOT_DISTANCE		(2*FIELD_HEIGHT/5)	// Start shooting, when this near to the player
#define AI_MIN_SHOOT_DISTANCE		(6*SHIP_LENGTH)		// Don't shoot if very close
#define AI_SHOOT_BEHIND_DISTANCE	50.0		// How far behind the player an enemy still may start a weapon.
#define AMOUNT_INITIAL_ENEMIES		3		// Size of group
#define ENEMY_MULTIPLY_DIVISOR		10		// larger --> more rarely additional enemies will spawn
#define ENEMY_BASE_SHOOT_INTERVAL_US	(1000000*3)	//*3 How often a blue enemy fires
#define ENEMY_TIER_SHOOT_FACTOR		9		// How much shooting interval increases with enemy's level
#define MIN_WARP_ENEMIES		4		// Player warps around without killing, additional enemies appear
#define MOTHERSHIP_SHOOT_FACTOR		19.0		// How much shooting interval increases with enemy's level
#define MOTHERSHIP_AGRESSIVENESS	9
#define MOTHERSHIP_PRE_AIM_OFFSET	17.0		// Effective: * SPEED_Y
#define MOTHERSHIP_SCORE		100.0		// How much Resource a mothership drops (bonus bubble)

#define ENEMY_RENDER_SIZE_FACTOR	1.2		// Visually scales the models
#define	ENEMY_SIZE_FACTOR		3.0		// Collision detection and size on screen
#define FORMATION_OFFSET		10.0		// How far away the next enemy of a group will be placed

#define EXPLOSION_PARTICLE_COUNT	36
#define EXPLOSION_DURATION_US		450000		//500000
#define EXPLOSION_RADIUS		17.5		//20.0

#define BONUS_BUBBLE_LIFETIME_US	(1000000*15)	// How long until a Resource bubble decays
#define BONUS_BUBBLE_ROTATION_US	1000000		// Base speed to be multiplied with  bubble_size
#define BONUS_BUBBLE_MIN_RADIUS		0.0
#define BONUS_BUBBLE_MAX_RADIUS		25.0

#define INITIAL_RANDOM_SEED		93		// The random seed

#define BONUS_FACTOR_SCORE		10		// Multipliers
#define BONUS_FACTOR_DISTANCE		10
#define BONUS_FACTOR_SPEED		25
#define BONUS_FACTOR_BEST_RESOURCE	5
#define BONUS_FACTOR_HIT_RATIO		25
#define BONUS_FACTOR_ENEMIES		1000

#if CHEAT_MODE
#define CHEAT_RESOURCE_FACTOR		20.0		// Increased Resource gain
#define NEW_ENEMY_DISTANCE_DIVISOR	1.0		//1 Mothership (yellow) may appear at distance 4
#else
#define CHEAT_RESOURCE_FACTOR		1.0		// Normal Resource gain
#define NEW_ENEMY_DISTANCE_DIVISOR	1000		//...? Describes when the next class of enemy may spawn
#endif


#endif
//EOF
