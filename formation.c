//formation.h
/******************************************************************************
* FORMATIONS
******************************************************************************/

#include <math.h>

#include "formation.h"
#include "main.h"
#include "game.h"
#include "enemy.h"


/* POSITIONS OF RANKS (Formation Layout)
 *
 * Enemies may detach from the formation or they may be killed. In this case,
 * the gap in the formation needs to be refilled by enemies still left in the
 * formation. In order to have the front line being occupied as long as
 * possible, the positions ("ranks") within the formation are placed in a
 * symmetrical way, starting to "fill" every new row from its center:
 *
 * 	WANTED LAYOUT					f ... first in row
 * row	at  width == 5			+1		m ... max.ranks/row - 1
 * 9				+2	24 m, f		+n ... delta ranks/row
 * 8			+3	23 m		22 f
 * 7		+4	21 m		19 f		20
 * 6		18 m		16		15 f		17
 * 5	14 m		12		10 f		11		13
 * 4	+5	9 m		7		6 f		8
 * 3		   +4	m 5		3 f		4
 * 2			   +3	2 m		1 f
 * 1	+y (1 line)		   +2	0 m, f		FRONT starting at
 *	 |						position 0
 *	 +-- +x (1 tab)
 */
int create_formation_positions( formation_t* f, int formation_width )
{
	int i;

	int row            = 0;
	int new_row_rank   = 0;
	int first_row_rank = 0;
	int row_growth     = 0;	// Prevent "uninitialized" warning

	int x, row_index;

debugf(DEBUG_FORMATION_CREATION, "create_formation_positions():");
if ((formation_width < 1) || (formation_width > MAX_FORMATION_SIZE)) {
	printf("PANIC: formation_width (%d) out of bounds\n", formation_width);
	exit(1);
}

	for( i = 0 ; i < MAX_FORMATION_RANKS ; i++ ) {

		/* DETERMINE CURRENT ROW */

		/* Check for row change. We start with new_row_rank == 0, so
		 * this block will be executed at the very begin, preparing the
		 * loop for the first row.
		 */
		if (i == new_row_rank) {	// Keep track of row

debugf(DEBUG_FORMATION_CREATION, "\n");

			++row;		// Next row, move up
					// First row starts with  row == 1

			/* Counting from the bottom, we start increasing
			 *  row_growth  until we reach the middle at
			 *  (row_growth + 1) == formation_width
			 * from then  row_growth  will shrink again
			 * until it reaches 0.
			 */
			if (row <= formation_width) {	// 1st (bottom) half?

				row_growth = row;	// Grow
			}
			else {
				--row_growth;		// Shrink

				if (row_growth == 0) {

					// Last row reached, return nr ranks
/* EXIT FUNCTION */			return i;
				}
			}

			// Store index to first rank of the next row
			new_row_rank += row_growth;	// Index to next first

			// Keep the index of the current first ("center") rank
			// for later calculations
			first_row_rank = i;		// Keep first
		}


		/* CALCULATE RANK COORDINATES */

		row_index = i - first_row_rank;		// Position in row

		// Starting centered in odd, offset in even rows
		x = !(row & 1);

		/* Calculate the position of the current rank within the row
		 */
		x += round(
			( row_index
			+ (row & 1)		// First is offset in odd rows
			)
			/ 2			// Rounded down
		);

		// Stretch formation, so rows can be offset half a position
		x *= 2;				// Stretch formation
		x -= !(row & 1);		// Half left for even rows

		/* Mirror first rank in odd rows, second rank in even rows
		 * If row is odd, mirror (1,) 3, 5, 7, ...
		 * Even rows: mirror 2, 4, 6, ...
		 */
		if ((row_index & 1) == (row & 1)) {

			x *= (-1);		// Mirror every second rank
		}


		/* ASSIGN VALUES TO RANK */

		f->ranks[i].occupied_by = NULL;

		f->ranks[i].coordinate.x = x;	// Int coords for finding..
		f->ranks[i].coordinate.y = row;	// ..neighbouring positions

debugf(DEBUG_FORMATION_CREATION, " %d(%d|%d)", i, x, row);

		f->ranks[i].position.x = FORMATION_OFFSET * x;
		f->ranks[i].position.y = FORMATION_OFFSET * (row - 1);

#if CURVED_FORMATIONS
		f->ranks[i].position.y -=
			row * FORMATION_OFFSET	// Move forward half a row
			* sin( M_PI * fabs(x)	// according to hor. position
			/ (FORMATION_OFFSET*formation_width/2))
		;
#endif
		f->ranks[i].position.z = 0;
	} // for i MAX_current_rankS

debugf(DEBUG_FORMATION_CREATION, "\n");

	return MAX_FORMATION_RANKS;	// If too many ranks, return max
}


// FORMATION RANK HIERARCHY ///////////////////////////////////////////////////

/* find_rank()
 * Searches the formation for a rank entry with the coordinates equal to those
 * given and returns the index to the array  ranks[]  of the formation.
 */
int find_rank( formation_t* formation, coordinate_t required, int nr_ranks )
{
	int i;
	coordinate_t test;

	for( i = 0 ; i < nr_ranks ; i++ ) {

		test = formation->ranks[i].coordinate;

		if ((test.x == required.x) &&  (test.y == required.y)) {

debugf(DEBUG_FORMATION_RANKS, " %d", i);

			return i;
		}
	}
debugf(DEBUG_FORMATION_RANKS, " nil");
	return -1;
}

/* create_formation_ranks()
 *
 * Calculates the neighbouring ranks and stores them in  fillfrom_index[] .
 * The values are used to determine if the "front man" in the formation was
 * killed and the gap has to be filled.
 *
 * I am using the index rather than a pointer for easier debugging/tweaking.
 *
 * "fillfrom order"
 * Firstly, the enemy at the rank behind the gap ("1", rank.coordinate.y + 2)
 * should fill it. If the back rank is empty, the rank behind and towards the
 * center of the formation is used, and so on:
 *
 * 					e
 * 				e		e
 * 			e		e		e
 * 		e		e		e		e
 *	e		e		e		1		e
 *		e	 	e		2		3
 *			e 		4		Gap
 *			 	e		e
 *					e
 *
 *
 * 					e
 * 				e		e
 * 			e		e		e
 * 		e		e		e		e
 *	e		e		e		e		e
 *		e	 	e		1		e
 *			e 		2		3
 *			 	(4)		Gap
 *					e
 *
 * In case (4) the rank is not used for refilling the gap, because the path
 * would cross the center. We want the outer regions to be refilled with
 * priority in order to keep the front line filled as long as possible.
 *
 * The "fillfrom"-rank indices will be stored in an array of every rank,
 * holding the neighbours in order of precedence.
 *
 * The current implementation might use slightly different orders of  fillfrom
 * entries.
 */
void create_formation_ranks( formation_t* f, int nr_ranks )
{
	int i, j;
	int x, y;
	coordinate_t c;

debugf(DEBUG_FORMATION_RANKS, "create_formation_ranks():\n");

	for( i = 0 ; i < nr_ranks ; i++ ) {

		// Clear the entries, not all will be set
		for( j = 0 ; j < NR_FILLFROM_RANKS ; j++ ) {
			f->ranks[i].fillfrom_index[j] = (-1);
		}

		x = f->ranks[i].coordinate.x;
		y = f->ranks[i].coordinate.y;

debugf(DEBUG_FORMATION_RANKS, " %d (%d|%d)\t<-- ", i, x, y);

		j = 0;		// Start with  fillfrom_index == 0

		c = f->ranks[i].coordinate;	// Above
		c.y -= -2;
		f->ranks[i].fillfrom_index[j++] = find_rank( f, c, nr_ranks );

		c = f->ranks[i].coordinate;	// Diagonal above, inwards
		c.x += (x == 0) ? -1 : -1 * sgn(x) ;
		c.y -= -1;
		f->ranks[i].fillfrom_index[j++] = find_rank( f, c, nr_ranks );

		if (abs(x) > 1) {
			c = f->ranks[i].coordinate;	// "Inward"
			c.x += -2 * sgn(x) ;
			f->ranks[i].fillfrom_index[j++]
				= find_rank( f, c, nr_ranks );
		}

		c = f->ranks[i].coordinate;	// Diagonal above, outwards
		c.x += (x == 0) ? +1 : +1 * sgn(x) ;
		c.y -= -1;
		f->ranks[i].fillfrom_index[j++] = find_rank( f, c, nr_ranks );

debugf(DEBUG_FORMATION_RANKS, "\n");

	}
}


/* create_formation()
 * Returns size (nr. available ranks) of formation
 */
int create_formation(
	game_state_t* GS,
	formation_t* formation,
	int top_tier,
	int formation_width,
	int formation_index,
	int nr_formations,
	real_t offset_y
	)
{
	formation_t* f = formation;

	f->position.x = f->velocity.x =
	f->position.y = f->velocity.y =
	f->position.z = f->velocity.z = 0;

	// Base position and velocity in respect to the whole wave

	f->position.x
		= (int)(formation_index * FIELD_WIDTH/3)
		% (int)(FIELD_WIDTH)
		- FIELD_WIDTH/2
		+ FIELD_WIDTH/6	// center the positions 0,1,2
	;

	f->position.y
		= GS->ship.position.y
		+ FIELD_HEIGHT * 0.4 //...0.4
		+ FIELD_HEIGHT * 0.6
		* (real_t) formation_index
		/ (real_t) nr_formations
		//...* level_length(GS)
		+ FIELD_HEIGHT * offset_y
	;

#if FREEZE_ENEMIES
	f->velocity.x = 0;
	f->velocity.y = 0;
#else
	f->velocity.x = SPEED_X * (0.25 + 0.25 * top_tier);
	f->velocity.y
		= SPEED_Y * 0.5
		- SPEED_Y
		* (top_tier/2.0 + (real_t)GS->current_level/6.0)
		/ 2
	;
#endif

	int ret = create_formation_positions( f, formation_width );
	create_formation_ranks( f, ret );

	return ret;
}


/* create_formation_enemies()
 * Fills a newly created formation with actual enemies
 */
void create_formation_enemies(
	game_state_t* GS,
	formation_t* formation,
	int top_tier,
	int amount
	)
{
	int i, new_tier;
	formation_t* f = formation;

	// Create enemies to fill the formation
	for( i = 0 ; i < amount ; i++ ) {

		// Adjust tier according to rank, decreasing with rank
		new_tier = max(
			TIER_1,
			top_tier - (f->ranks[i].coordinate.y - 1)
		);
//((i+1)/2) )
		f->ranks[i].occupied_by
			= add_enemy(
				GS,
				new_tier,
				add_vector(
					f->position,
					f->ranks[i].position
				),
				f->velocity,
				f
			)
		;
	}

}


/* next_free_rank()
 */
formation_rank_t* next_free_rank( formation_t* formation )
{
	int i;

	if (formation != NULL) {

		for( i = 0 ; i < formation->nr_ranks ; i++ ) {

			if (formation->ranks[i].occupied_by == NULL) {

				return &(formation->ranks[i]);
			}
		}
	}

	return NULL;
}


/* formation_turn_around()
 * When an enemy leaves the game area, the whole formation is triggered to
 * turn into the other direction
 */
void formation_turn_around( game_state_t* GS, formation_t* formation )
{
	formation->velocity.x *= (-1);

#ifdef DISABLED_CODE
	int i;
	enemy_t* e;

	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {

		e = &(GS->enemies[i]);

		if (e->active && (e->formation == formation)) {

			e->velocity.x *= (-1);
		}
	}
#endif
}


/* advance_formation()
 * Check if ranks need to be refilled and command according enemy to switch
 * to  ai_mode == RANK_TRANSIT .
 */
void advance_formation(
	program_state_t* PS,
	game_state_t* GS,
	formation_t* formation
	)
{
	formation_t* f = formation;

	f->position.x += f->velocity.x * PS->tick_fraction_s;
	f->position.y += f->velocity.y * PS->tick_fraction_s;
	f->position.z += f->velocity.z * PS->tick_fraction_s;

	normalize_position_y( GS, &(f->position) );

	//... set enemy positions calculated from this formation's position

	int i, j, from_index;
	enemy_t* e;

	for( i = 0 ; i < f->nr_ranks - 1 ; i++ ) {

		if (f->ranks[i].occupied_by == NULL) {	// Needs refill?
/////////////////
for( j = 0 ; j < NR_FILLFROM_RANKS ; j++ ) {

	from_index = f->ranks[i].fillfrom_index[j];

	if (from_index >= 0) {			// Direction exists

		e = f->ranks[from_index].occupied_by;

		if ((e != NULL) && (e->ai_mode == FORMATION)) {

			e->target_rank = &(f->ranks[i]);
			e->target_rank->occupied_by = e;

			e->current_rank->occupied_by = NULL;

			e->ai_mode = RANK_TRANSIT;
			e->rank_change_start_us = PS->game_time_us;

			j = NR_FILLFROM_RANKS;	// Break loop
		}

	}
}
/////////////////
		}
	}
}


//EOF
