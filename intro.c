//intro.c
/******************************************************************************
* INTRO SCREEN
******************************************************************************/

#include <SDL/SDL_opengl.h>

#include "intro.h"
#include "main.h"
#include "hud.h"
#include "gl_helpers.h"
#include "ui.h"


#define ROTATION_DURATION	(1000000 * 10)
#define RADIUS			100.0
#define RADIUS_OFFSET		0.1
#define CAMERA_ANGLE		-75.0
#define CAMERA_DISTANCE		15.0
#define CAMERA_OFFSET		13.0	// Positivie values move image up
#define FOG_DENSITY		0.02


void draw_grid( program_state_t* PS )
{
	int w = PS->window_width;
	int h = PS->window_height;
	microtime_t T = PS->current_time_us;

	GLfloat x, y, z;
	GLfloat a, r;

	GLfloat camera_distance = CAMERA_DISTANCE;
	GLfloat camera_rotation
		= (T % ROTATION_DURATION)
		/ (GLfloat)ROTATION_DURATION
		* 360.0
	;

	GLfloat aspect = (GLfloat)w / (GLfloat)h;

	glViewport( 0, 0, w, h );
	glClearColor( 0.0f, 0.0f, 0.1f, 1.0f );	// Background dark blue, opaque
	glClearDepth( 1.0f );                   // Background depth to farthest
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Draw the 3D scene
	glMatrixMode( GL_PROJECTION );		// Setup perspective mode
	glLoadIdentity();
	gluPerspective(
		75.0f,				// fovy   Field of View, y-Axis
		aspect,				// Aspect ratio of the screen
		0.1f,				// zNear
		3000.0				// zFar
	);

	glMatrixMode( GL_MODELVIEW );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

#ifdef INTRO_USES_FOG
	GLfloat fog_color[4] = { 0,0,0, 0 };	// Fog of depth
	glEnable( GL_FOG );
	glFogi( GL_FOG_MODE, GL_EXP2 );
	glFogfv( GL_FOG_COLOR, fog_color );
	glFogf( GL_FOG_DENSITY, FOG_DENSITY );
	glHint( GL_FOG_HINT, GL_NICEST );
#endif

	glLoadIdentity();			// Reset
	glTranslatef( 0, CAMERA_OFFSET, -camera_distance );
	glRotatef( CAMERA_ANGLE, 1,0,0 );
	glRotatef( camera_rotation, 0,0,1 );

	glPointSize( 1.0 );
	glColor3f( 1.0, 1.0, 1.0 );

	int A, R;
	GLfloat start_radius = (T % 100000) / -100000.0 + 0.01;

	glBegin( GL_POINTS );
	for( A = 0 ; A < 360 ; A += 5 ) {

		a = A * DEG_TO_RAD;

		for( R = 0 ; R < 100 ; R++ ) {

			r = start_radius + R * 100.0/RADIUS;

			x = cos(a) * r;
			y = sin(a) * r;
			z = -RADIUS / sqrt(r + RADIUS_OFFSET);

			glVertex3f( x, y, z );
		}
	}
	glEnd();
	glPointSize( 1 );

#ifdef INTRO_USES_FOG
	glDisable( GL_FOG );
#endif
}

void draw_overlay( program_state_t* PS )
{
	int w = PS->window_width;
	int h = PS->window_height;
	microtime_t T = PS->current_time_us;

	GLfloat R, G, B;

	glViewport( 0, 0, w, h );		// Using screen coordinates

	glMatrixMode( GL_PROJECTION );		// Setup 2D mode
	glLoadIdentity();
	gluOrtho2D( 0, w, 0, h );

	glMatrixMode( GL_MODELVIEW );
	glDisable( GL_DEPTH_TEST );
	glLoadIdentity();

	color_from_hue( (T % 6000000) / 6000000.0, &R, &G, &B );
	glColor3f( R, G, B );

	glLineWidth( 2 );
	glBegin( GL_LINES );
		glVertex2i( 0, 2*h/3 );
		glVertex2i( w, 2*h/3 );
	glEnd();
	glLineWidth( 1 );

	hud_printf( PS,
		w/2,
		2*h/3 + PS->line_height/2,
		ALIGN_CENTER,
		0xFFFFFF,
		PROGRAM_NAME
	);

	hud_printf( PS,
		w/2,
		2*h/3 - 3*PS->line_height/2,
		ALIGN_CENTER,
		0x888888,
		PROGRAM_VERSION
	);
}

void draw_intro_frame( program_state_t* PS )
{
	draw_grid( PS );
	draw_overlay( PS );
}


//EOF
