//hud.c
/******************************************************************************
* DRAW HUD
*******************************************************************************
* Rendering text as a 2D overlay over an OpenGL scene, used for drawing a
* head-up display and debug info.
*
* snprintf
******************************************************************************/

#include <stdarg.h>
#include <SDL/SDL_opengl.h>

#include "hud.h"
#include "main.h"
#include "gl_helpers.h"
#include "ui.h"
#include "draw_frame.h"
#include "scene.h"
#include "game.h"

#if DEBUG_MEMORY
#include <malloc.h>	// Memory usage debug info
#endif


/******************************************************************************
* HUD HELPER FUNCTIONS
******************************************************************************/

/* hud_printf()
 * Renders a string onto the screen. Does the real work, while the other
 * draw_hud_element_*() functions only create strings from a variety of
 * parameters and types.
 */
void hud_printf(
	program_state_t* PS,
	int x,
	int y,
	alignment_t alignment,
	int color,
	char* format_string,
	...
	)
{
	va_list	argp;
	char* s;

	GLuint	new_texture = 0;
	int w, h;


	// Obtain pointer to first argument after  format_string
	va_start( argp, format_string );

	// Format the variable number of parameters
	vasprintf( &s, format_string, argp );

	va_end(argp);


	// Render the string to a bitmap and create an OpenGL texture

	new_texture = text_to_texture(
		PS->font,
		color,
		s,
		&w,
		&h
	);
	if (new_texture == 0) {
		error_quit( "hud element: text_to_texture() returned NULL" );
	}
	free( s );


	// Adjust position

	if (alignment & ALIGN_RIGHT)  x -= w;
	if (alignment & ALIGN_CENTER) x -= w/2;
	if (alignment & ALIGN_BOTTOM) y -= h;
	if (alignment & ALIGN_MIDDLE) y -= h/2;


	// Render the bitmap onto screen

	glPushMatrix();

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, new_texture );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor3ub( 255, 255, 255 );

	glLoadIdentity();
	glTranslatef( x, y, 0 );
	glBegin( GL_QUADS );
		glTexCoord2i( 0, 0 );  glVertex2i( 0, h );
		glTexCoord2i( 1, 0 );  glVertex2i( w, h );
		glTexCoord2i( 1, 1 );  glVertex2i( w, 0 );
		glTexCoord2i( 0, 1 );  glVertex2i( 0, 0 );
	glEnd();

	glDeleteTextures( 1, &new_texture );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );

	glPopMatrix();
}

void hud_print_time(
	program_state_t* PS,
	int x,
	int y,
	alignment_t alignment,
	int color,
	microtime_t micros
	)
{
	bool_t dot_visible = (PS->current_time_us % 1000000 < 800000);
	int ms = micros / 1000;
	int sec = ms / 1000;
	int min = sec / 60;
	sec -= 60 * min;
	char* s;

	// Format time to string with blinking colon
	asprintf(
		&s,
		(dot_visible) ? "T=%02d:%02d.%d" : "T=%02d %02d.%d",
		min,
		sec,
		(ms % 1000) / 100
	);

	hud_printf( PS, x, y, alignment, color, s );
	free( s );
}

#define LEVEL_QUAD_WIDTH	25
#define LEVEL_QUAD_HEIGHT	35
#define LEVEL_OFFSET_X		(PS->line_height + LEVEL_QUAD_WIDTH)
#define LEVEL_OFFSET_Y		(PS->window_height - 7*PS->line_height/2)
void hud_draw_level_nr( program_state_t* PS, game_state_t* GS )
{
	const int w = LEVEL_QUAD_WIDTH;
	const int h = LEVEL_QUAD_HEIGHT;
	const int y = LEVEL_OFFSET_Y;

	int number;
	GLfloat digit_x;
	bool_t draw_zero;

	int quad_x  = LEVEL_OFFSET_X - LEVEL_QUAD_WIDTH/2;


	number = GS->current_level;
	draw_zero = (number == 0);
	while (number > 9) {
		quad_x += LEVEL_QUAD_WIDTH/2;
		number /= 10;
	}
	number = GS->current_level;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, PS->textures.digits );

	glBegin( GL_QUADS );
	glColor4f( 1,1,1, 0.5 );
					// more centered vertically.
	while (number || draw_zero) {
		digit_x = ((number % 10) / 10.0);
		draw_zero = FALSE;	// Only draw the zero once

		glTexCoord2f( digit_x,     1 );	glVertex2i( quad_x,   y-h/2 );
		glTexCoord2f( digit_x+0.1, 1 );	glVertex2i( quad_x+w, y-h/2 );
		glTexCoord2f( digit_x+0.1, 0 );	glVertex2i( quad_x+w, y+h/2 );
		glTexCoord2f( digit_x,     0 );	glVertex2i( quad_x,   y+h/2 );

		quad_x -= LEVEL_QUAD_WIDTH;
		number /= 10;
	}
	glEnd();

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );

	hud_printf(
		PS,
		LEVEL_OFFSET_X,
		LEVEL_OFFSET_Y - LEVEL_QUAD_HEIGHT - PS->line_height/2,
		ALIGN_CENTER,
		0x808080,
		"WAVE"
	);

	if (GS->show_wave_until_us > PS->current_time_us) {

		int w = PS->window_width;
		int h = PS->window_height;

		real_t fade
			= (real_t)(GS->show_wave_until_us - PS->current_time_us)
			/ NEXT_WAVE_NOTICE_DURATION
		;

		fade = sin( fade * 1.444 );

		int color
			= ( (int)(fade * (real_t)0x44) * 0x10000 )
			+ ( (int)(fade * (real_t)0xAA) * 0x100 )
			+ ( (int)(fade * (real_t)0x11) )
		;

		hud_printf(
			PS,
			w/2, 4*h/6, ALIGN_CENTER, color, "Wave %d",
			GS->current_level
		);
	}
}


/******************************************************************************
* TOP PANEL
******************************************************************************/

/* draw_hud_top_panel_background()
 * Renders a transparent rectangle
 */
void draw_hud_panel_background( program_state_t* PS, alignment_t alignment )
{
	int w = PS->window_width;
	int h = PS->window_height;
	int panel_height = 2 * PS->line_height;


	glEnable( GL_BLEND );
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (alignment & ALIGN_TOP) {
		glColor4f( 1,1,1, 0.2 );	//... Why does this color look
		glBegin( GL_QUADS );		//... as bright as the below??
			glVertex2i( 0, h );
			glVertex2i( w, h );
			glVertex2i( w, h - panel_height );
			glVertex2i( 0, h - panel_height );
		glEnd();

		glColor3ub( 128, 128, 128 );
		glBegin( GL_LINES );
			glVertex2i( 0, h - panel_height );
			glVertex2i( w, h - panel_height );
		glEnd();
	}

	if (alignment & ALIGN_BOTTOM) {
		glColor4f( 1,1,1, 0.1 );	//... Why does this color look
		glBegin( GL_QUADS );		//... as bright as the above??
			glVertex2i( 0, panel_height );
			glVertex2i( w, panel_height );
			glVertex2i( w, 0 );
			glVertex2i( 0, 0 );
		glEnd();

		glColor3ub( 128, 128, 128 );
		glBegin( GL_LINES );
			glVertex2i( 0, panel_height );
			glVertex2i( w, panel_height );
		glEnd();
	}

	glDisable( GL_BLEND );
}


// RESOURCE //////////////////////////////////////////////////////////////////

void draw_hud_resource( program_state_t* PS, game_state_t* GS )
{
	int	i, j;
	real_t	x, y;
	GLfloat	R, G, B;

	real_t	bar_width, step_size;
	real_t	resource;
	int	resource_color;
	int	hit_points = calculate_hit_points( GS->current_resource );

	microtime_t Tc = PS->current_time_us;

	int w = PS->window_width;
	int h = PS->window_height;

	// In cheat mode, after the numbers are output as digits, ..
	// ..we use an adjusted Resource value to get a nice bar size
	resource
		= GS->current_resource
#if CHEAT_MODE
		/ CHEAT_RESOURCE_FACTOR
#endif
	;


	/* CALCULATE BASE COLOR indicating level of resource */

	color_from_hue(
		// The following ratio is manually tweaked to give nice..
		// ..results up to 10K. Perhaps using log2 would be better?
		fmin( 1.0, sqrt(GS->current_resource) / 100.0 ),
		&R, &G, &B
	);

	resource_color				// Pack the float values into..
		= ((int)(R * 255) << 16)	// ..a handy int.
		+ ((int)(G * 255) << 8)
		+ ((int)(B * 255) << 0)
	;


	/* RENDER RESOURCE as TEXT (ALIGN_TOP Panel) */

	hud_printf(
		PS,
		PS->window_width/2,
		h - 3*PS->line_height/2,
		ALIGN_CENTER,
		resource_color,
		"RESOURCE: %d (%d)",
		GS->current_resource,
		GS->best_resource
	);


	/* CALCULATE BLINK COLOR */

	// From here on, everything rendered might be blinking

	if (resource >= 200) {
		/* Reuse R, G and B from above, no change needed */
	}
	else {	if      (resource < 50)	i = 150000;
		else if (resource < 100)	i = 225000;
		else if (resource < 150)	i = 350000;
		else 					i = 500000;

		GLfloat f = fabs((int)(Tc % i) - i/2) / ((GLfloat)i/2);
		R = R * f;
		G = G * f;
		B = B * f;

		// Additionally blink white, when Resource critically low
		if ((resource < 50) && (Tc % 150000 < 50000)) {
			R = G = B = 1.0;
		}
	}
	resource_color				// Pack the float values into..
		= ((int)(R * 255) << 16)	// ..a handy int.
		+ ((int)(G * 255) << 8)
		+ ((int)(B * 255) << 0)
	;


	/* RESOURCE BAR */

	// Calculate size of the bar
	bar_width = sqrt(resource) * 10.0;	//... Perhaps we should use  log2 ?
	step_size = round( bar_width / hit_points );

	glBegin( GL_LINES );
	glColor3f( R, G, B );

	for( i = 0 ; i < 5 ; i++ ) {
		for( j = 0 ; j < hit_points ; j++ ) {

			x = (w - bar_width)/2 + (real_t)j * step_size;
			y = h - 3*PS->line_height + i*2;

			glVertex2i( x + 1.0,             y );
			glVertex2i( x - 1.0 + step_size, y );
		}
	}
	glEnd();
}


// CURRENT ENEMIES ////////////////////////////////////////////////////////////

#define ICON_SIZE	12

void draw_current_enemy(
	program_state_t* PS,
	int tier,
	int x,
	int y,
	int size,
	bool_t filled
	)
{
	real_t R, G, B;

	x	= PS->window_width
		- PS->line_height
		- x * (ICON_SIZE + 3)/2
		+ (ICON_SIZE + 2) / 2
	;

	y	= PS->window_height
		- 3*PS->line_height
		- y * (ICON_SIZE + 5)
	;

	get_tier_color( tier, &R, &G, &B );
	R *= filled ? 0.7 : 0.3 ;
	G *= filled ? 0.7 : 0.3 ;
	B *= filled ? 0.7 : 0.3 ;
	glColor3f( R, G, B );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glBegin( GL_POLYGON );
	switch (tier) {
	case MOTHERSHIP_TIER:
		glVertex2i( x, y + size/2 );
		glVertex2i( x + size/2, y );
		glVertex2i( x, y - size/2 );
		glVertex2i( x - size/2, y );
		break;

	default:
		glVertex2i( x,          y + 2*size/3 );
		glVertex2i( x - size/2, y - 1*size/3 );
		glVertex2i( x + size/2, y - 1*size/3 );
	}
	glEnd();

	// Draw outline
	get_tier_color( tier, &R, &G, &B );
	glColor3f( R, G, B );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glBegin( GL_POLYGON );
	switch (tier) {
		case MOTHERSHIP_TIER:
			glVertex2i( x, y + size/2 );
			glVertex2i( x + size/2, y );
			glVertex2i( x, y - size/2 );
			glVertex2i( x - size/2, y );
			break;

		default:
			glVertex2i( x,          y + 2*size/3 );
			glVertex2i( x - size/2, y - 1*size/3 );
			glVertex2i( x + size/2, y - 1*size/3 );
	}
	glEnd();
}

void hud_current_enemies( program_state_t* PS, game_state_t* GS )
{
	int i, j, s;
	int nr_enemies;
	int nr_bubbles;
	int x = 0;
	int y = 0;

	int enemies_tens, enemies_ones;
	int bubbles_tens, bubbles_ones;

	glEnable( GL_BLEND );
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POLYGON_SMOOTH );

	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

	glLineWidth( PS->window_width / 512 );

	for( i = 0 ; i < NR_TIERS ; i++ ) {

		nr_enemies = GS->nr_active_enemies[i];
		nr_bubbles = GS->nr_active_bonus_bubbles[i];

		enemies_tens  = nr_enemies / 10;
		enemies_ones  = nr_enemies % 10;
		nr_enemies = enemies_tens + enemies_ones;	// Number of actual icons

		bubbles_tens  = nr_bubbles / 10;
		bubbles_ones  = nr_bubbles % 10;
		nr_bubbles = bubbles_tens + bubbles_ones;	// Number of actual icons

		x = 0;

		for( j = 0 ; j < nr_enemies ; j++ ) {
			s = (j < enemies_tens) ? ICON_SIZE : ICON_SIZE * 0.75 ;
			draw_current_enemy( PS, i, ++x, y, s, /*filled*/TRUE );
		}
		for( j = 0 ; j < nr_bubbles ; j++ ) {
			s = (j < bubbles_tens) ? ICON_SIZE : ICON_SIZE * 0.75 ;
			draw_current_enemy( PS, i, ++x, y, s, /*filled*/FALSE );
		}

		if ((nr_enemies > 0) || (nr_bubbles > 0)) {
			++y;
		}
	}

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	//GL_FILL
	glLineWidth( 1 );

	glDisable( GL_POLYGON_SMOOTH );
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_BLEND );
}

// WEAPON STATE ///////////////////////////////////////////////////////////////

/* get_weapon_color()
 * Returns two colors (enabled/disabled) for rendering the weapon status.
 * If blinking is indicated for the weapon, the returned colors will be bright
 * "blink colors" for a certain fraction in a looping time interval.
 */
void hud_weapon_state(
	program_state_t* PS,
	game_state_t* GS,
	int weapon_nr,
	char* caption,
	int x,
	int y
	)
{
	int color_enabled;
	int color_disabled;

	weapon_t* wpn = &(GS->ship.weapons[weapon_nr]);

	microtime_t elapsed_time
		= PS->current_time_us
		- wpn->blink_start_us
	;

	bool_t highlight
		= (elapsed_time % HUD_BLINK_SPEED > 2*HUD_BLINK_SPEED/3 )
	;

	if (wpn->blink_until_us > PS->current_time_us) {

		color_enabled
			= highlight
			? COLOR_ENABLED_LO
			: COLOR_BLINK_HI
		;

		color_disabled
			= highlight
			? COLOR_DISABLED_LO
			: COLOR_BLINK_HI
		;
	}
	else {	color_enabled  = COLOR_ENABLED_HI;
		color_disabled = COLOR_DISABLED_HI;
	}

	hud_printf(
		PS,
		x, y,
		ALIGN_LEFT,
		(wpn->enabled ? color_enabled : color_disabled ),
		caption
	);
}

/* hud_score_summary()
 * Display a detailed explanation how score was calculated.
 * Details need to be manually adjusted the algorithm in
 * game.c: calculate_total_score()
 */
void hud_score_summary( program_state_t* PS, game_state_t* GS )
{
	int w = PS->window_width;
	int h = PS->window_height;
	int x = w/2;
	int y = 3*h/5;
	int i = 6;	// Initial upwards offset, half of text shown

	int score         = GS->score.current;
	int total_score   = calculate_total_score( PS, GS );

	score_info_t* si = &(GS->score);


	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		"SCORE                               %7d",
		score
	);

	--i;	// Omit one line

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		"HIT RATIO     %7d%%  x%7d  = %7d",
		si->hit_ratio,
		score,
		score * si->hit_ratio/100
	);

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		"SPEED BONUS   %7d%%  x%7d  = %7d",
		si->speed,
		score,
		score * si->speed/100
	);

	--i;	// Omit one line

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		"BEST RESOURCE  %7d  x%7d  = %7d",
		si->best_resource,
		BONUS_FACTOR_BEST_RESOURCE,
		BONUS_FACTOR_BEST_RESOURCE * si->best_resource
	);

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		"DISTANCE       %7d  x%7d  = %7d",
		si->distance,
		BONUS_FACTOR_DISTANCE,
		BONUS_FACTOR_DISTANCE * si->distance
	);

	--i;	// Omit one line

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xFFFFFF,
		"                             TOTAL: %7d",
		total_score
	);

	--i;

	hud_printf(
		PS,
		x,
		y + (i--) * (PS->line_height),
		ALIGN_CENTER, 0xAAAAAA,
		( total_score > GS->score.high_score ? "High Score!" : "High Score: %d (Wave %d)" ),
		GS->score.high_score,
		GS->score.high_score_level
	);
}


/******************************************************************************
* DRAW HUD
******************************************************************************/

/* draw_hud()
 * Main HUD rendering function, calculates some statistics needed only for
 * the display
 */
void draw_hud( program_state_t* PS, game_state_t* GS )
{
	int w = PS->window_width;
	int h = PS->window_height;

	setup_gl_2D( w, h );

	hud_draw_level_nr( PS, GS );

	draw_hud_panel_background( PS, ALIGN_TOP );


	/* TIME, HIT RATIO, SCORE */

	hud_print_time(
		PS,
		1*PS->line_height,
		h - 3*PS->line_height/2,
		ALIGN_LEFT,
		COLOR_TEXT_LO,
		PS->game_time_us
	);
	hud_printf(
		PS,
		7*PS->line_height,
		h - 3*PS->line_height/2,
		ALIGN_LEFT,
		COLOR_TEXT_LO,
		"HR:%.0f%%",
		calculate_hit_ratio(GS) * 100.0
	);
	hud_printf(
		PS,
		11.5*PS->line_height,
		h - 3*PS->line_height/2,
		ALIGN_LEFT,
		COLOR_TEXT_HI,
		"SCORE:%d",
		GS->score.current
	);


	/* WEAPON STATE */

	hud_weapon_state(
		PS, GS, WEAPON_LASER_1, "[LSR]",
		w - 12 * PS->line_height,
		h - 3*PS->line_height/2
	);
	hud_weapon_state(
		PS, GS, WEAPON_LASER_2, "[DBL]",
		w - 9 * PS->line_height,
		h - 3*PS->line_height/2
	);
	hud_weapon_state(
		PS, GS, WEAPON_ROUNDSHOT, "[RSH]",
		w - 6 * PS->line_height,
		h - 3*PS->line_height/2
	);
	hud_weapon_state(
		PS, GS, WEAPON_AUTOFIRE, "[AF] ",
		w - 3 * PS->line_height,
		h - 3*PS->line_height/2
	);


	/* RESOURCE */

	draw_hud_resource( PS, GS );


	/* ENEMIES */

	hud_current_enemies( PS, GS );


	/* BOTTOM MENU (MAIN MENU, PAUSE) */

	if (PS->run_mode & (RM_PAUSE | RM_MAIN_MENU)) {

		draw_hud_panel_background( PS, ALIGN_BOTTOM );

		hud_printf(
			PS,
			w/2,
			PS->line_height/2,
			ALIGN_CENTER,
			0xFFFFFF,

			(PS->run_mode == RM_PAUSE)
			? "PAUSE - Press [RETURN] to continue, [ESC] to exit the game."
			: "Press [RETURN] to start a new game, [ESC] to exit the game."
		);
	}


	/* AFTER LIFE SCORE */

	if (PS->run_mode & (RM_MAIN_MENU | RM_PAUSE)) {

		hud_score_summary( PS, GS );
	}
}


/******************************************************************************
* DEBUG INFO
******************************************************************************/

void debug_times(
	program_state_t* PS,
	char* format_string,
	microtime_t us,
	int* line_count
	)
{
	hud_printf(
		PS,
		PS->line_height,
		PS->line_height * (*line_count)++,
		(ALIGN_LEFT | ALIGN_MIDDLE),
		COLOR_TEXT_LO,
		format_string,
		us
	);
}

void draw_debug( program_state_t* PS, game_state_t* GS )
{
#if DEBUG

	int w = PS->window_width;
	int h = PS->window_height;

	int line_count = 3;

	setup_gl_2D( w, h );

#if DEBUG_MEMORY
	struct mallinfo mi = mallinfo();

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"Mem: uordblks(%d) delta(%d)",
		(int) mi.uordblks,
		(int) (PS->initial_TAS - mi.uordblks)
	);

	if (PS->initial_TAS == -1) {
		PS->initial_TAS = mi.uordblks;
	}
#endif

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"Tframe(%.3fms) F/s(%.3f)",
		PS->average_frame_time / 1000.0,
		1000000.0 / PS->average_frame_time
	);

#if DEBUG_TIMES
	debug_times(
		PS, "highest_frame_time = %lld",
		PS->highest_frame_time, &line_count
	);
	debug_times(
		PS, "lowest_frame_time  = %lld",
		PS->lowest_frame_time, &line_count
	);
	debug_times(
		PS, "pause_since_us  = %lld",
		PS->pause_since_us - PS->program_start_us, &line_count
	);
	debug_times(
		PS, "current_time_us = %lld",
		PS->current_time_us - PS->program_start_us, &line_count
	);
	debug_times(
		PS, "game_time_us = %lld",
		PS->game_time_us, &line_count
	);
	debug_times(
		PS, "game_start_us   = %lld",
		PS->game_start_us - PS->program_start_us, &line_count
	);
#endif

#if DEBUG_SHIP
	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"ship.position = ( %g | %g | %g )",
		GS->ship.position.x,
		GS->ship.position.y,
		GS->ship.position.z
	);

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"ship.velocity = ( %g | %g | %g )",
		GS->ship.velocity.x,
		GS->ship.velocity.y,
		GS->ship.velocity.z
	);

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"camera_speed_pitch(%f) ",
		GS->ship.camera_speed_pitch
	);
#endif

#if DEBUG_GAME_STATISTICS
	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"En Route(%d) Fired(%d) Missed(%d)",
		GS->shots_en_route,
		GS->shots_fired,
		GS->shots_missed
	);

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"Lsr(%d) Enm(%d %d %d %d) Xpl(%d) BBl(%d %d %d %d)",
		GS->nr_active_lasers,
		GS->nr_active_enemies[0],
		GS->nr_active_enemies[1],
		GS->nr_active_enemies[2],
		GS->nr_active_enemies[3],
		GS->nr_active_explosions,
		GS->nr_active_bonus_bubbles[0],
		GS->nr_active_bonus_bubbles[1],
		GS->nr_active_bonus_bubbles[2],
		GS->nr_active_bonus_bubbles[3]
	);

	calculate_total_score( PS, GS );
	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"Speed bonus: %d",
		GS->score.speed
	);

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"nr_active_enemies(%d)",
		GS->nr_active_enemies_total
	);

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"add_enemy_beyond_y(%.2f) distance(%.2f)",
		GS->add_enemy_beyond_y,
		GS->ship.position.y - GS->add_enemy_beyond_y
	);
#endif

	hud_printf(
		PS,
		PS->line_height,
		PS->line_height*line_count++,
		ALIGN_LEFT | ALIGN_MIDDLE,
		COLOR_TEXT_LO,
		"%s %s",
		PROGRAM_NAME,
		PROGRAM_VERSION
	);
#endif
}


//EOF
