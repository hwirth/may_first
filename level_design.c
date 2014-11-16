//level_design.c
/******************************************************************************
* LEVEL DESIGN
*******************************************************************************
* Functions related to the rules of the game, like preparing/resetting the
* game engine, adding enemies, etc.
******************************************************************************/

#include "level_design.h"
#include "main.h"
#include "game.h"
#include "enemy.h"
#include "formation.h"


/******************************************************************************
* BLACK HOLE
******************************************************************************/

void generate_black_hole( game_state_t* GS )
{
	black_hole_t* bh = &(GS->black_hole);

	bh->position.x
		= (	rand_int( &(bh->random_seed) )
			% (int)FIELD_WIDTH
		)
		- FIELD_WIDTH / 2
	;
	bh->position.y = GS->ship.position.y + FIELD_HEIGHT;

	bh->velocity.x = BLACK_HOLE_SPEED_X;
	bh->velocity.y = BLACK_HOLE_SPEED_Y;
}


#if BLACK_HOLES_SUCK
void blackhole_attracts_ship( game_state_t* GS )
{
	real_t dx;	// x portion of distance to the Black Hole
	real_t dx1;	// dx reduced to the x portion of the unity vector
	real_t a;	// Angle of camera rotation, so that we look into
			// the Black Hole
	real_t df;	// Distance Factor; 0..1 when near the BH,
			// 1, when outside of the BH's radius
	real_t distance = GS->ship.distance_to_black_hole;

	df = fmin(1.0, distance / BLACK_HOLE_RADIUS_SHIP);
	df = (1.0 - sqrt(df));

	dx = GS->black_hole.position.x - GS->ship.position.x;
	dx1 = dx / distance;

	a = df * (90.0 - acos(dx1) * RAD_TO_DEG);

	GS->black_hole.camera_rotation.x = ((dx > 0) ? +a : -a );
	GS->black_hole.camera_rotation.y = ((dx > 0) ? +a : -a );
	GS->black_hole.camera_rotation.z = a;

	GS->black_hole.ship_translation.z = df * -50.0;
}
#endif


/******************************************************************************
* WAVES (Level Design v2)
******************************************************************************/

void create_wave( game_state_t* GS, int level, int* formation_index )
{
	int fi = *formation_index;

	int i, n;
	int a1, a2, a3, a4;	// Amount per tier
	int s1, s2, s3, s4;	// Width (formation size) per tier

	formation_t* f;

/* Amount and size per level
 *
 * Wave:  1     2     3     4     5     6     7     8     9     10
 * Tier1:  1     1,1   4     4,4   9     9,9   16    16,16 25    25,25 ...
 * Tier2:  -     -     1     1,1   4     4,4   9     9,9   16    16,16 ...
 * Tier3:  -     -     -     -     1     1,1   4     4,4   9     9,9   ...
 * Tier4:  -     -     -     -     -     -     1     1,1   4     4,4   ...
 *
 * Levels: '--1--'     '--3--'     '--5--'     '--7--'     '--9--'     ...
 *               '--2--'     '--4--'     '--6--'     '--8--'     '--10--'
 *
 * This function is usually called several times like shown under "Waves"
 * above, to make up more nicely populated levels.
 */
	// Calculate, which formations per tier to create..
	const int fmax = sqrt( MAX_FORMATION_RANKS );
	s1 = a1 = min( fmax, max(0, (level+1)/2) );  a1 *= a1;  // ..and the..
	s2 = a2 = min( fmax, max(0, (level-1)/2) );  a2 *= a2;  // ..amount..
	s3 = a3 = min( fmax, max(0, (level-3)/2) );  a3 *= a3;  // ..of enemies
	s4 = a4 = min( fmax, max(0, (level-5)/2) );  a4 *= a4;

	// Clamp sizes
	s1 = a1 = min( 2, s1 );  a1 *= a1;
	s3 = a3 = min( 4, s3 );  a3 *= a3;
	s4 = a4 = min( 2, s4 );  a4 *= a4;

	n = !(level & 1);		// Odd level numbers: n=1, ..
	++n;				// .. even: n=2 (*)

#ifdef DISABLED_CODE
s1 = a1 = 1;
s2 = a2 = 0;
s3 = a3 = 0;
s4 = a4 = 0;
n = level;
#endif

	int fi_max = n * (s1 + s2 + s3 + s4);	// Number of formatios to create

	for( i = 0 ; i < n ; i++ ) {
		if (fi < MAX_FORMATIONS) {
			f = &(GS->formations[fi]);
			f->nr_ranks = create_formation( GS, f, TIER_1, s1, fi, fi_max, 0 );
			create_formation_enemies( GS, f, TIER_1, a1 );
			++fi;
		}
	}
	for( i = 0 ; i < n ; i++ ) {
		if (fi < MAX_FORMATIONS) {
			f = &(GS->formations[fi]);
			f->nr_ranks = create_formation( GS, f, TIER_2, s2, fi, fi_max, 0 );
			create_formation_enemies( GS, f, TIER_2, a2 );
			++fi;
		}
	}
	for( i = 0 ; i < n ; i++ ) {
		if (fi < MAX_FORMATIONS) {
			f = &(GS->formations[fi]);
			f->nr_ranks = create_formation( GS, f, TIER_3, s3, fi, fi_max, 0 );
			create_formation_enemies( GS, f, TIER_3, a3 );
			++fi;
		}
	}
	for( i = 0 ; i < n ; i++ ) {
		if (fi < MAX_FORMATIONS) {
			f = &(GS->formations[fi]);
			f->nr_ranks = create_formation( GS, f, TIER_4, s1, fi, fi_max, 0 );
			create_formation_enemies( GS, f, TIER_4, a4 );
			++fi;
		}
	}

	*formation_index = fi;
}


/******************************************************************************
* POPULATE LEVEL
******************************************************************************/

/* Level Design v3
 *
 * 1. Determine amount and tiers of enemies
 * 2. Break up groups of types into formations of 1(*), 4, 9, 16 enemies
 * 	2.1. Ratio of formations sizes? (50% single, rest in groups?)
 * 3. Distribute formations throughout the level
 * 	3.1. Evenly? Weighted to group size?
 * 	3.2. Adjust level size to amount of enemies?
 *
 * Determining amount and tiers of enemies
 *   Level	A	B	C	D
 *   1		7
 *   2		8	1
 *   3		9	2
 *   4		10	3
 *   5		11	4
 *   6		12	5
 *   7		13	6
 *   8		14	7
 *   9		15	8	1
 *   10		16	9	2
 *   11		17	10	3
 *   12		18	11	4
 *   13		19	12	5
 *   14		20	13	6
 *   15		21	14	7
 *   16		22	15	8	1
 *   17		23	16	9	2
 *   18		24	17	10	3
 *   19		25	18	11	4
 *   20		26	19	12	5
 *   21		27	20	13	6
 *   22		28	21	14	7
 */

void create_wave2(
	game_state_t* GS,
	int tier,
	int amount,
	int nr_units,
	int* formation_index
) {
	int fsize;
	int fi = *formation_index;
	formation_t* f;
	real_t offset_y;

	offset_y = (GS->current_level > 1) ? NEXT_WAVE_PAUSE_OFFSET : 0 ;

	if (amount < 1) return;

	for( fsize = MAX_FORMATION_SIZE ; fsize > 1 ; fsize-- ) {
		while (amount > 2*fsize*fsize) {
			f = &(GS->formations[fi]);
			f->nr_ranks = create_formation(
				GS, f, tier, fsize, fi, nr_units, offset_y
			);
			create_formation_enemies( GS, f, tier, fsize*fsize );

			++fi;
			amount -= fsize*fsize;
		}
	}

	// Create remaining amount as single enemies
	for( fsize = 1 ; amount > 0 ; amount-- ) {
		f = &(GS->formations[fi]);
		f->nr_ranks = create_formation(
			GS, f, tier, fsize, fi, nr_units, offset_y
		);
		create_formation_enemies( GS, f, tier, fsize*fsize );

		++fi;
		--amount;
	}

	*formation_index = fi;
}

/* calc_nr_units()
 * nr_units  (total amount of formations/single enemies) needs to be
 * known before actual creation for positioning
 */

#define LT1	+8	//+6
#define LT2	+1	//-1
#define LT3	-4	//-8
#define	LT4	-9	//-15

int calc_nr_units( int level )
{
	int i;
	int nr_units = 0;
	int nr_enemies
		= max( 0, level+LT1 )
		+ max( 0, level+LT2 )
		+ max( 0, level+LT3 )
		+ max( 0, level+LT4 )
	;

	for( i = MAX_FORMATION_SIZE ; i > 1 ; i-- ) {
		while (nr_enemies > 2*i*i) {
			++nr_units;
			nr_enemies -= i*i;
		}
	}
	nr_units += nr_enemies;	// Left over enemies will be units of one.

	return nr_units;
}

void create_units( game_state_t* GS )
{
	int L = GS->current_level;
	int nr_units = calc_nr_units( L );
	int formation_index = 0;

	create_wave2( GS, TIER_1, L+LT1, nr_units, &formation_index );
	create_wave2( GS, TIER_2, L-LT2, nr_units, &formation_index );
	create_wave2( GS, TIER_3, L+LT3, nr_units, &formation_index );
	create_wave2( GS, TIER_4, L+LT4, nr_units, &formation_index );
}


/******************************************************************************
* LEVEL CONTROL
******************************************************************************/

void advance_to_next_level( program_state_t* PS, game_state_t* GS )
{
	int i, j, k;

	formation_rank_t* r;

	// Reset formations
	for( i = 0 ; i < MAX_FORMATIONS ; i++ ) {

		GS->formations[i].nr_ranks = 0;

		for( j = 0 ; j < MAX_FORMATION_RANKS ; j++ ) {

			r = &( GS->formations[i].ranks[j] );

			r->occupied_by = NULL;

			for( k = 0 ; k < NR_FILLFROM_RANKS ; k++ ) {
				r->fillfrom_index[k] = -1;
			}
		}
	}

	//...! record score

	//...GS->shots_fired	= 0;
	//...GS->shots_missed	= 0;
	//...GS->best_resource	= 0;

	++GS->current_level;

	GS->show_wave_until_us
		= PS->current_time_us
		+ NEXT_WAVE_NOTICE_DURATION
	;

#if TEST_LEVELS
	int L = GS->current_level;
	formation_t* f = &(GS->formations[0]);
	f->nr_ranks = create_formation( GS, f, TIER_1, L, 0, 1 );
	create_formation_enemies( GS, f, TIER_1, L*L );
#elif LEVEL_DESIGN_V2
	int L = GS->current_level;
	int formation_index = 0;
	create_wave( GS, L, &formation_index );
	create_wave( GS, L+1, &formation_index );
#else
	create_units( GS );

#endif
}

void prepare_first_level( program_state_t* PS, game_state_t* GS )
{
	GS->shots_fired    = 0;
	GS->shots_missed   = 0;
	GS->shots_en_route = 0;
	GS->best_resource  = 0;
	GS->enemies_killed = 0;


	GS->current_resource = INITIAL_RESOURCE * CHEAT_RESOURCE_FACTOR;
	GS->score.current    = 0;

	GS->ship.position = vector( 0,0,0 );
	GS->ship.weapons[WEAPON_LASER_1].enabled = TRUE;

	GS->next_recharge_beyond_y = RECHARGE_DISTANCE;
	GS->next_recharge_after_us = RECHARGE_TIME;

	GS->add_enemy_beyond_y = calculate_enemy_beyond_y( GS );
	GS->nr_warp_enemies = MIN_WARP_ENEMIES;	// Next time, the player warps around,
					// ..spawn one new enemy.

	GS->black_hole.random_seed = 0;
	generate_black_hole( GS );

	GS->current_level = 0;
	advance_to_next_level( PS, GS );	// Increases level, spawns enemies
}


/******************************************************************************
* WARP AROUND WITHOUT KILLING ANY ENEMIES
******************************************************************************/

real_t calculate_enemy_beyond_y( game_state_t* GS )
{
	return GS->ship.position.y + FIELD_HEIGHT * 1.6;
}


void player_warped_around( game_state_t* GS )
{
#if WARP_AROUND_SPAWNS_ENEMY
	int i;
	formation_t* f = NULL;

	for( i = 0 ; i < MAX_FORMATIONS ; i++ ) {
		if (GS->formations[i].nr_ranks == 0) {
			f = &(GS->formations[i]);
			break;
		}
	}

	if (f != NULL) {
		int tier = TIER_1;
		int fsize = 1 + sqrt( GS->nr_warp_enemies );
		//... check, if in range
		int offset_y = 0;
		f->nr_ranks = create_formation(
			GS, f, tier, fsize, 1, 1, offset_y
		);
		create_formation_enemies( GS, f, tier, GS->nr_warp_enemies );
		//... Check if formation is removed when last enemy is killed
	}

	GS->add_enemy_beyond_y = calculate_enemy_beyond_y( GS );

	// Increase number of new enemies in case, the player
	// does not kill within the next "warp"
	GS->nr_warp_enemies += 2;
#endif
}

//EOF
