//debug.c
/******************************************************************************
* DEBUG HELPER FUNCTIONS
******************************************************************************/

#include <stdio.h>
#include <SDL/SDL_opengl.h>


/* gl_debug() - Checks for OpenGL Errors and prints message to screen
 */
void gl_debug()
{
	int err = glGetError();
	if (err) {
		printf("glGetError returned %04X.\n", err);
		//exit( -1 );
	}

} // gl_debug


//EOF

