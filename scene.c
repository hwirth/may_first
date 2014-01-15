//scene.c
/******************************************************************************
* DRAW SCENE
*******************************************************************************
* Rendering of the game world, according to data in  game_state .
******************************************************************************/

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "scene.h"
#include "main.h"
#include "game.h"
#include "hud.h"
#include "world.h"	// remove_explosion()
#include "ui.h"
#include "gl_helpers.h"


// SCENE HELPER FUNCTIONS /////////////////////////////////////////////////////

/* set_color()
 * Helper function to let ships be drawn in their shaded base color or
 * highlighted when they are hit
 */
void set_color(
	real_t R,
	real_t G,
	real_t B,
	real_t intensity,
	real_t depth_factor,
	bool_t flashing
	)
{
	real_t f = depth_factor;

	if (flashing) {
		R = G = B = 1.0;
	}
	else {
		f *= intensity;
	}

	glColor3f( R*f, G*f, B*f );
}

#if LINE_MARKERS
/* draw_distance_marker()
 * Showing the distance as numbers (in the middle of the field)
 */
void draw_distance_marker(
	program_state_t* PS,
	GLfloat y,		// Position relative to the ship
	int line_position_y,	// Coordinate of the labeled line
	real_t depth_factor
	)
{
	const int w = LABEL_QUAD_WIDTH;
	const int h = LABEL_QUAD_HEIGHT;
	int number;

	GLfloat z, digit_x;
	GLfloat quad_x = LABEL_OFFSET_X - LABEL_QUAD_WIDTH/2.0;

	bool_t draw_zero;


	number = line_position_y / 100;
	draw_zero = (number == 0);
	// Find out, how long the number is, preparing;
	// quad_x as offset to the first (rightmost) digit
	while (number > 9) {
		quad_x += LABEL_QUAD_WIDTH/2.0;
		number /= 10;
	}
	number = line_position_y / 100;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, PS->textures.digits );


	glBegin( GL_QUADS );
	glColor4f( LABEL_COLOR, depth_factor );

	y += LABEL_OFFSET_Y;	// Cheat offset to make numbers appear
					// more centered vertically.
	z = GRID_DELTA_Z + 0.1;

	while (number || draw_zero) {

		digit_x = ((number % 10) / 10.0);
		draw_zero = FALSE;	// Only draw the zero once

	glTexCoord2f( digit_x,     1 );	glVertex3f( quad_x,   y-h/2, z );
	glTexCoord2f( digit_x+0.1, 1 );	glVertex3f( quad_x+w, y-h/2, z );
	glTexCoord2f( digit_x+0.1, 0 );	glVertex3f( quad_x+w, y+h/2, z );
	glTexCoord2f( digit_x,     0 );	glVertex3f( quad_x,   y+h/2, z );

		quad_x -= LABEL_QUAD_WIDTH;
		number /= 10;
	}

	glEnd();


	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
}
#endif


// DRAW SCENE PARTS ///////////////////////////////////////////////////////////

/* draw_scene_grid()
 */
void draw_scene_grid(
	program_state_t* PS,
	game_state_t* GS,
	real_t camera_distance,
	real_t camera_rotation,
	real_t offset_y
	)
{
	int ix, iy, line_position_y;
	real_t x, y, z, grid_offset_y;
	real_t depth_factor;
	ship_t* s = &(GS->ship);

	grid_offset_y = -(s->position.y);			// Warp
	grid_offset_y
		-= trunc(grid_offset_y / FIELD_STEP_Y / GRID_CELL_SIZE)
		* FIELD_STEP_Y
		* GRID_CELL_SIZE
	;

	glLoadIdentity();					// Reset
	glTranslatef(
		(s->position.x / SHIP_MAX_X) * CAMERA_OFFSET_X,
		-offset_y,
		-camera_distance
	);
	glRotatef( camera_rotation, 1,0,0 );
	glTranslatef(			// Move the field into world
		-s->position.x,
		grid_offset_y - offset_y + CAMERA_OFFSET_Y,
		0.0
	);

	glPointSize( 1.5 );

	glBegin( GL_POINTS );

	for( y=FIELD_MIN_Y, iy=0 ; y <= FIELD_MAX_Y ; y+=FIELD_STEP_Y, iy++ ) {

		line_position_y			// y-coordinate of this line
			= (int)(
				s->position.y
				+ grid_offset_y
				+ GRID_CHEAT_OFFSET_Y
			)
			+ iy
			* FIELD_STEP_Y
		;

		depth_factor
			= 1.0
			- sqrt( fabs(
				y / FIELD_MAX_Y / GRID_DEPTH_FACTOR
				)
			)
		;

#ifdef DISABLED_CODE
		glEnd();
		glPointSize(
			(GLfloat)PS->window_width
			/ 600.0				//...
			* depth_factor
		);
		glBegin( GL_POINTS );
#endif

		for(	x = FIELD_MIN_X, ix = 0
		;	x <= FIELD_MAX_X
		;	x += FIELD_STEP_X, ix++
		) {

			if ((ix % GRID_CELL_SIZE) && (iy % GRID_CELL_SIZE)) {
				glColor3f(
					0.0,
					0.25*depth_factor,
					depth_factor*0.9
				);
			}
			else {
				glColor3f(
					0.0,
					depth_factor*0.8,
					depth_factor*1.0
				);
			}

#if LINE_MARKERS
			if (line_position_y % 100 == 0) {
	#if LINE_LABELS
				glEnd();	// Points

				draw_distance_marker(
					PS,
					y,
					line_position_y,
					depth_factor
				);

				glBegin( GL_POINTS );
	#endif
				glColor3f(
					depth_factor,
					depth_factor,
					depth_factor
				);
			}
#endif

#if DRAW_BLACK_HOLES
			// Black Hole
			real_t distance = sqrt(
				  (x - GS->black_hole.position.x)
				* (x - GS->black_hole.position.x)
				+ (line_position_y - GS->black_hole.position.y)
				* (line_position_y - GS->black_hole.position.y)
			);

			if (distance != 0) {	// Function describing the hole
				z = -25000 / (distance*distance);
			}

			// Black hole appears only, if player near
			z = fmin(
				0,
				z * (
					1.0
#if BLACK_HOLES_OPEN_ON_APPROACH
					- s->distance_to_black_hole
#else
					- distance
#endif
					/ (FIELD_HEIGHT/6)
				)
			);
#else
			z = 0;
#endif
			glVertex3f( x, y, z + GRID_DELTA_Z );
		}
	}

	glEnd();
}

/* draw_scene_ship()
 */
void draw_scene_ship(
	program_state_t* PS,
	game_state_t* GS,
	real_t camera_distance,
	real_t camera_rotation,
	real_t offset_y
	)
{
	int i;
	microtime_t T = PS->current_time_us;
	ship_t* s = &(GS->ship);

	glLoadIdentity();
	glTranslatef(
		(s->position.x / SHIP_MAX_X) * CAMERA_OFFSET_X,
		-offset_y,
		-camera_distance
	);
	glRotatef( camera_rotation, 1,0,0 );
	glTranslatef(
		0,
					// Move ship into world position
		-offset_y + CAMERA_OFFSET_Y,	// Move down on screen
		0
	);

	if (PS->run_mode & (RM_RUNNING | RM_PAUSE)) {
		glPushMatrix();

#if BLACK_HOLES_SUCK
		glRotatef( GS->black_hole.camera_rotation.z, 0,0,-1 );
#endif

		// Simulated roll
#if FIRE_DIAGONALLY
		glRotatef( fabs(s->velocity.x)/10,  1,0,0 );
#endif

		glRotatef(       s->velocity.x*10,  0,1,0 );	// Fwd axis

#if FIRE_DIAGONALLY
		glRotatef(       s->velocity.x*5,   0,0,-1 );
#endif

#if BLACK_HOLES_SUCK
		glTranslatef(
			0,
			0,
			(GLfloat) GS->black_hole.ship_translation.z
		);
#endif

		i = (s->indicate_hit_until_us > T) ? 255 : 0 ;

		glBegin( GL_TRIANGLES );

			glColor3ub( i, 96, 192 );
			glVertex3f(  0,  0,  1 );
			glVertex3f(  2, -1,  0 );
			glVertex3f(  0,  3,  0 );

			glColor3ub( i, 64, 128 );
			glVertex3f(  0,  0, -1 );
			glVertex3f(  2, -1,  0 );
			glVertex3f(  0,  3,  0 );

			glColor3ub( i, 128, 255 );
			glVertex3f(  0,  0,  1 );
			glVertex3f( -2, -1,  0 );
			glVertex3f(  0,  3,  0 );

			glColor3ub( i, 96, 192 );
			glVertex3f(  0,  0, -1 );
			glVertex3f( -2, -1,  0 );
			glVertex3f(  0,  3,  0 );

			glColor3ub( i, 48, 96 );
			glVertex3f(  0,  0,  1 );
			glVertex3f(  0,  0, -1 );
			glVertex3f(  2, -1,  0 );

			glColor3ub( i, 32, 64 );
			glVertex3f(  0,  0,  1 );
			glVertex3f(  0,  0, -1 );
			glVertex3f( -2, -1,  0 );

		glEnd();

		glPopMatrix();		// Undo roll
	}
}

/* draw_scene_enemies()
 */
void draw_scene_enemies( program_state_t* PS, game_state_t* GS )
{
	int i;
	microtime_t Tc = PS->current_time_us;
	microtime_t Te = PS->game_time_us;

	real_t R, G, B;
	real_t depth_factor;
	GLfloat rotation;

	bool_t flashing;

	const real_t h = sqrt(0.75);
	real_t h2;
	real_t Fs;				// Size factor

	enemy_t* e;

	for( i = 0 ; i < MAX_ENEMIES ; i++ ) {

		e = &(GS->enemies[i]);

		if (e->active) {

			depth_factor
				= 1.0
				- fabs( e->position.y - GS->ship.position.y )
				/ (FIELD_HEIGHT/2)
			;

			if (depth_factor < 0) {
				depth_factor = 0;
			}
			else if (depth_factor > 1.0) {
				depth_factor = 1.0;
			}

			rotation
				= (real_t)(Te % 1000000) / 1e6
#if ENEMIES_ROTATE_INDIVIDUALLY
				* (i*M_PI*30.0),
#else
				* 360.0
#endif
			;

			glPushMatrix();
			glTranslatef( e->position.x, e->position.y, 0 );
			glRotatef( rotation, 0,0,1 );

			R = e->color.R;
			G = e->color.G;
			B = e->color.B;

			flashing = (e->indicate_hit_until_us > Tc);

			//... use glScale instead of Fs
			Fs = enemy_size( GS, e ) * ENEMY_RENDER_SIZE_FACTOR;

			if (e->tier == MOTHERSHIP_TIER) {

				// "Pumping" movement
				h2 = Fs * (1 + cos(Te / 123000.0)*0.5) / 2;

				glBegin( GL_TRIANGLES );

				set_color( R,G,B, 0.4, depth_factor, flashing );
				glVertex3f(    0,     0, Fs );
				glVertex3f(+Fs/2, +Fs/2, h2 );
				glVertex3f(-Fs/2, +Fs/2, h2 );

				set_color( R,G,B, 0.3, depth_factor, flashing );
				glVertex3f(    0,     0,  0 );
				glVertex3f(+Fs/2, +Fs/2, h2 );
				glVertex3f(-Fs/2, +Fs/2, h2 );

				set_color( R,G,B, 0.6, depth_factor, flashing );
				glVertex3f(    0,     0, Fs );
				glVertex3f(-Fs/2, +Fs/2, h2 );
				glVertex3f(-Fs/2, -Fs/2, h2 );

				set_color( R,G,B, 0.5, depth_factor, flashing );
				glVertex3f(    0,    0,   0 );
				glVertex3f(-Fs/2, +Fs/2, h2 );
				glVertex3f(-Fs/2, -Fs/2, h2 );

				set_color( R,G,B, 0.8, depth_factor, flashing );
				glVertex3f(    0,     0, Fs );
				glVertex3f(-Fs/2, -Fs/2, h2 );
				glVertex3f(+Fs/2, -Fs/2, h2 );

				set_color( R,G,B, 0.7, depth_factor, flashing );
				glVertex3f(    0,     0,  0 );
				glVertex3f(-Fs/2, -Fs/2, h2 );
				glVertex3f(+Fs/2, -Fs/2, h2 );

				set_color( R,G,B, 1.0, depth_factor, flashing );
				glVertex3f(    0,     0, Fs );
				glVertex3f(+Fs/2, -Fs/2, h2 );
				glVertex3f(+Fs/2, +Fs/2, h2 );

				set_color( R,G,B, 0.9, depth_factor, flashing );
				glVertex3f(    0,     0,  0 );
				glVertex3f(+Fs/2, -Fs/2, h2 );
				glVertex3f(+Fs/2, +Fs/2, h2 );

				glEnd();
			}
			else {
				glBegin( GL_TRIANGLES );

				set_color( R,G,B, 0.6, depth_factor, flashing );
				glVertex3f(    0,        0, +h*Fs );
				glVertex3f(+Fs/2,  -h/3*Fs,     0 );
				glVertex3f(    0, h*2/3*Fs,     0 );

				set_color( R,G,B, 0.8, depth_factor, flashing );
				glVertex3f(    0,        0, +h*Fs );
				glVertex3f(    0, h*2/3*Fs,     0 );
				glVertex3f(-Fs/2,  -h/3*Fs,     0 );

				set_color( R,G,B, 1.0, depth_factor, flashing );
				glVertex3f(    0,        0, +h*Fs );
				glVertex3f(-Fs/2,  -h/3*Fs,     0 );
				glVertex3f(+Fs/2,  -h/3*Fs,     0 );

				glEnd();
			}
			glPopMatrix();
		}
	}
}

/* draw_scene_lasers()
 */
void draw_scene_lasers( program_state_t* PS, game_state_t* GS )
{
	int i;
	real_t x, y, z;
	real_t depth_factor, distance;
	laser_beam_t* l;

	for( i = 0 ; i < MAX_LASER_BEAMS ; i++ ) {

		l = &(GS->laser_beams[i]);

		if (l->active) {

			depth_factor
				= 1.0
				- (l->position.y - GS->ship.position.y)
				/ (FIELD_HEIGHT/2)
			;

			if (l->owner < 0) {	// Player
				glColor3f( 0, depth_factor, 0 );
			}
			else {
				glColor3f( depth_factor, depth_factor, 0 );
			}

			glLineWidth(
				(GLfloat)PS->window_width
				/ 256.0				//...
				* depth_factor
			);

			glBegin( GL_LINES );

				glVertex3f(
					l->position.x,
					l->position.y,
					l->position.z
				);

				x = GS->laser_beams[i].velocity.x;
				y = GS->laser_beams[i].velocity.y;
				z = GS->laser_beams[i].velocity.z;

				distance = sqrt( x*x + y*y + z*z );

				x = GS->laser_beams[i].position.x
					+ x / distance * LASER_LENGTH_FACTOR
				;

				y = GS->laser_beams[i].position.y
					+ y / distance * LASER_LENGTH_FACTOR
				;

				z = GS->laser_beams[i].position.z
					+ z / distance * LASER_LENGTH_FACTOR
				;

				glVertex3f( x, y, z );

			glEnd();
		}
	}
	glLineWidth( 1.0 );
}

/* draw_scene_explosions()
 */
void draw_scene_explosion(
	program_state_t* PS,
	game_state_t* GS,
	explosion_t* e
	)
{
	int i;
	real_t x, y, z;
	real_t radius, angle;
	real_t depth_factor;

#if SPATIAL_EXPLOSIONS
	real_t inclination;
#endif

	if (PS->game_time_us - e->start_time_us
		> EXPLOSION_DURATION_US)
	{
		remove_explosion( GS, e );
	}
	else {
		depth_factor
			= 1.0
			- (e->position.y - GS->ship.position.y)
			/ (FIELD_HEIGHT/2)
		;

		glPointSize( depth_factor * 4 );
		glBegin( GL_POINTS );

		for( i = 0 ; i < EXPLOSION_PARTICLE_COUNT ; i++ ) {

			radius = (real_t)(
					PS->game_time_us
					- e->start_time_us
				)
				/ EXPLOSION_DURATION_US
			;

			angle = (real_t) i
				/ EXPLOSION_PARTICLE_COUNT
				* 2*M_PI
			;
#if SPATIAL_EXPLOSIONS
			inclination
				= (rand() % 360)
				* DEG_TO_RAD
			;
#endif
			x = e->position.x
				+ cos(angle)
				* radius
				* EXPLOSION_RADIUS
#if SPATIAL_EXPLOSIONS
				* sin( inclination )
#endif
			;

			y = e->position.y
				+ sin(angle)
				* radius
				* EXPLOSION_RADIUS
#if SPATIAL_EXPLOSIONS
				* sin( inclination )
#endif
			;

			z = 0.0//...e->position.z
#if SPATIAL_EXPLOSIONS
				+ cos( inclination )
				* radius
				* EXPLOSION_RADIUS/4
#endif
			;

#if COLORED_EXPLOSIONS
#ifdef DISABLED_CODE
			radius		// Repurposing radius
				= 1.0
				- (1-radius)*(1-radius)
			;
#endif
			glColor3f(
				(1-radius) * 1,
				(1-radius) * fmax( 0, 1-2*radius ),
				(1-radius) * fmax( 0, 1-9*radius )
			);
#else
			glColor3f( 1-radius, 0.5-radius/2, 0 );
#endif
			glVertex3f( x, y, z );
		}

		glEnd();
	}
}

void draw_scene_explosions( program_state_t* PS, game_state_t* GS )
{
	int i;
	explosion_t* e;

	for( i = 0 ; i < MAX_EXPLOSIONS ; i++ ) {

		e = &(GS->explosions[i]);

		if (e->active) {
			draw_scene_explosion( PS, GS, e );
		}
	}
}

/* draw_scene_bonus_bubbles()
 */
void draw_scene_bonus_bubbles( program_state_t* PS, game_state_t* GS )
{
	int i;
	real_t depth_factor;
	real_t size;
	real_t angle;
	real_t x, y;
	const real_t z = /*//...GRID_DELTA_Z - 0.01;*/ 0.0;
	real_t time_angle;

	bonus_bubble_t* b;

	glLineWidth( 1.5 );

	for( i = 0 ; i < MAX_BONUS_BUBBLES ; i++ ) {

		b = &(GS->bonus_bubbles[i]);

		if (b->active) {

			if (PS->game_time_us - b->start_time_us
				> BONUS_BUBBLE_LIFETIME_US)
			{
				remove_bonus_bubble( GS, b );
			}
			else {
				depth_factor
					= 1.0
					- (b->position.y - GS->ship.position.y)
					/ (FIELD_HEIGHT/2)
				;

				size = bubble_size( b );

				time_angle
					= (real_t)(
						PS->game_time_us
						% (int)(
							BONUS_BUBBLE_ROTATION_US
							* size
						)
					)
					/ BONUS_BUBBLE_ROTATION_US
					/ size
					* 360.0
				;

				glPointSize( 5 * depth_factor );
				set_color(
					b->color.R,
					b->color.G,
					b->color.B,
					1.0,	// Intensity
					depth_factor,
					FALSE
				);

				glBegin( GL_POINTS );
				glVertex3f(
					b->position.x,
					b->position.y,
					z
				);
				glEnd();

				glBegin( GL_LINES );
				for( angle = 0 ; angle <  360 ; angle += 360/(12+2*size) ) {

					x = b->position.x
						+ cos( (time_angle + angle)
							* DEG_TO_RAD
						)
						* size
					;

					x = fmin( FIELD_MAX_X, fmax(FIELD_MIN_X, x) );

					y = b->position.y
						+ sin( (time_angle + angle)
							* DEG_TO_RAD
						)
						* size
					;

					glVertex3f( x, y, z );
				}
				glEnd();
			}
		}
	}

	glPointSize( 1 );
	glLineWidth( 1 );
}

#if BLACK_HOLES_DARKEN_SCREEN
/* draw_scene_blackhole_darkness()
 * Darkens the rendered scene according to distance to the black hole
 * by creating a more or less transparent black quad infront of the scene.
 */
void draw_scene_blackhole_darkness( program_state_t* PS, game_state_t* GS )
{
        GLfloat darkness_factor
                = 1.0 - fmin(
                        1.0,
                        GS->ship.distance_to_black_hole
                        / BLACK_HOLE_RADIUS_DARKNESS
                )
        ;
        darkness_factor = sqrt( darkness_factor );
        if ((PS->run_mode & (RM_AFTER_LIFE | RM_MAIN_MENU))
        && (GS->ship.distance_to_black_hole <= BLACK_HOLE_RADIUS_DARKNESS))
        {
		darkness_factor = 1.0;
	}

        glMatrixMode( GL_PROJECTION );          // Setup 2D mode
        glLoadIdentity();
        gluOrtho2D(
                0, PS->window_width,            // Let coordinates match..
                0, PS->window_height            // ..screen resolution
        );
        glMatrixMode( GL_MODELVIEW );
        glDisable( GL_DEPTH_TEST );

        glLoadIdentity();

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );    //... Does this
        glColor4f( 0,0,0, darkness_factor );                    // work on many
        glBegin( GL_QUADS );                                    // computers?
                glVertex2i( 0, 0 );
                glVertex2i( 0, PS->window_height );
                glVertex2i( PS->window_width, PS->window_height );
                glVertex2i( PS->window_width, 0 );
        glEnd();
        glDisable( GL_BLEND );
}
#endif


// DRAW SCENE /////////////////////////////////////////////////////////////////

void show_background_image( program_state_t* PS, game_state_t* GS )
{
	int w = PS->window_width;
	int h = PS->window_height;
	const int bi = BACKGROUND_INTENSITY;
	GLfloat x, y;

	glMatrixMode( GL_PROJECTION );		// Setup 2D mode
	glLoadIdentity();
	gluOrtho2D( 0, w, 0, h );
	glMatrixMode( GL_MODELVIEW );
	glDisable( GL_DEPTH_TEST );
	glLoadIdentity();

	glEnable( GL_TEXTURE_2D );
	glColor3ub( bi, bi, bi );
	glBindTexture( GL_TEXTURE_2D, PS->textures.background );

	// Calculate offset for the texture
	x = GS->ship.position.x / PS->window_width;

	y	= GS->ship.camera_speed_pitch	// How much to move up/down
		/ -7.0				// Fraction of screen height
	;

	glBegin( GL_QUADS );
		glTexCoord2f( x+0, y+0 );	glVertex2i( 0, h );
		glTexCoord2f( x+1, y+0 );	glVertex2i( w, h );
		glTexCoord2f( x+1, y+1 );	glVertex2i( w, 0 );
		glTexCoord2f( x+0, y+1 );	glVertex2i( 0, 0 );
	glEnd();

	glDisable( GL_TEXTURE_2D );
}

/* draw_scene()
 * Visualizes the current  game_state .
 */
void draw_scene( program_state_t* PS, game_state_t* GS )
{
	real_t offset_y				// Adjust y-position of..
		= GS->ship.camera_speed_pitch	// ..the ship in respect..
		* CAMERA_PITCH_OFFSET_Y		// ..to current speed
	;

	real_t camera_distance
		= GS->camera.distance
		- GS->ship.camera_speed_pitch
		* CAMERA_DISTANCE_SPEED_FACTOR
	;

	real_t camera_rotation
		= GS->camera.angle
		+ GS->ship.camera_speed_pitch
		* CAMERA_ROTATION_ROLL_FACTOR	// degrees
	;

	/* Drawing the grid and the ship need almost the same transformations,
	 * except that the grid is "reused" every GRID_CELL_SIZE units
	 * traveled by the player.
	 */
	draw_scene_grid( PS, GS, camera_distance, camera_rotation, offset_y );

	// The ship routine creates new transformations
	draw_scene_ship( PS, GS, camera_distance, camera_rotation, offset_y );

	glTranslatef(			// Other objects need to be displayed..
		-GS->ship.position.x,	// ..in a position relative to the..
		-GS->ship.position.y,	// ..player's ship
		-GS->ship.position.z
	);

	// This leaves us with transformations set for drawing everything else:

	glEnable( GL_BLEND );
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_LINE_SMOOTH );

	draw_scene_enemies( PS, GS );
	draw_scene_lasers( PS, GS );
	draw_scene_explosions( PS, GS );
	draw_scene_bonus_bubbles( PS, GS );

	glDisable( GL_BLEND );
	glDisable( GL_LINE_SMOOTH );

#if BLACK_HOLES_DARKEN_SCREEN
	draw_scene_blackhole_darkness( PS, GS );
#endif
}


/* draw_game_frame()
 * Sets up OpenGL for 3D, then 2D rendering and calls the functions, that will
 * draw the game world and the HUD (overlay).
 */
void draw_game_frame( program_state_t* PS, game_state_t* GS )
{
	int w = PS->window_width;
	int h = PS->window_height;

	GLfloat aspect = (GLfloat)w / (GLfloat)h;

	glViewport( 0, 0, w, h );

	glClearColor( 0.0, 0.0, 0.05, 0.0 );	// Background blue and opaque
	glClearDepth( 1.0 );			// Background depth to farthest
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

#if SHOW_BACKGROUND
	show_background_image( PS, GS );
#endif

	// Draw the 3D scene
	glMatrixMode( GL_PROJECTION );		// Setup perspective mode
	glLoadIdentity();
	gluPerspective(
		45.0f,				// fovy   Field of View, y-Axis
		aspect,				// Aspect ratio of the screen
		0.1f,				// zNear
		30000.0f			// zFar
	);

	glMatrixMode( GL_MODELVIEW );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	glEnable( GL_BLEND );			// Enabling blending for..
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_POINT_SMOOTH );		// ..anti-aliasing.
	glEnable( GL_LINE_SMOOTH );
	//glEnable( GL_POLYGON_SMOOTH );	//... looks like polygon antialiasing isn't that simple

	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	//glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	draw_scene( PS, GS );			// Draw the world

	glDisable( GL_POINT_SMOOTH );
	glDisable( GL_LINE_SMOOTH );
	//glDisable( GL_POLYGON_SMOOTH );
	glDisable( GL_BLEND );

	draw_hud( PS, GS );			// Draw the overlay
}

//EOF
