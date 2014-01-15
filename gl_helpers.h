//gl_helpers.h
/******************************************************************************
* GL HELPERS
******************************************************************************/
#ifndef GL_HELPERS_H_PARSED
#define GL_HELPERS_H_PARSED


#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>


// PROTOTYES //////////////////////////////////////////////////////////////////

void setup_gl_2D( int window_width, int window_height );
void log_opengl_errors( void );

void color_from_hue( GLfloat hue, GLfloat* R, GLfloat* G, GLfloat* B );

GLuint text_to_texture( TTF_Font* font, int color, const char* text, int* result_w, int* result_h );
GLuint load_texture( char* file_name, int* result_w, int* result_h );


#endif
//EOF
