//gl_helpers.c
/******************************************************************************
* GL_HELPERS
******************************************************************************/

#include <math.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>

#include "gl_helpers.h"
#include "ui.h"


void log_opengl_errors( void )
{
	GLenum gl_error;

	do {
		gl_error = glGetError();
		fprintf( stderr, "%s\n", gluErrorString(gl_error));
	}
	while (gl_error != GL_NO_ERROR);
}

void setup_gl_2D( int window_width, int window_height )
{
	glMatrixMode( GL_PROJECTION );		// Setup 2D mode
	glLoadIdentity();
	gluOrtho2D(
		0, window_width,		// Let coordinates match..
		0, window_height		// ..screen resolution
	);
	glMatrixMode( GL_MODELVIEW );
	glDisable( GL_DEPTH_TEST );
}

void color_from_hue( GLfloat hue, GLfloat* R, GLfloat* G, GLfloat* B )
{
	GLfloat part = (6.0*hue - floor(6.0 * hue));

	if (hue < 1.0/6.0) {	// Red --> Yellow
		*R = 1;
		*G = part;
		*B = 0;
	}
	else if (hue < 2.0/6.0) {	// Yellow --> Green
		*R = 1.0 - part;
		*G = 1;
		*B = 0;
	}
	else if (hue < 3.0/6.0) {	// Green --> Cyan
		*R = 0;
		*G = 1;
		*B = part;
	}
	else if (hue < 4.0/6.0) {	// Cyan --> Blue
		*R = 0;
		*G = 1.0 - part;
		*B = 1;
	}
	else if (hue < 5.0/6.0) {	// Blue --> Magenta
		*R = part;
		*G = 0;
		*B = 1;
	}
	else {			// Magenta --> Red
		*R = 1;
		*G = 0;
		*B = 1.0 - part;
	}
}

GLuint text_to_texture(
	TTF_Font* font,
	int color,
	const char* text,
	int* result_w,
	int* result_h
	)
{
	GLuint texture;
	GLenum byte_order;

#ifdef TEXT_COLORS_WRONGLY
	SDL_Color sdl_color = {
		.r = (color & 0xFF0000) >> 16,
		.g = (color & 0x00FF00) >> 8,
		.b = (color & 0x0000FF) >> 0,
	};
#else
	SDL_Color sdl_color = {
		.b = (color & 0xFF0000) >> 16,
		.g = (color & 0x00FF00) >> 8,
		.r = (color & 0x0000FF) >> 0,
	};
#endif

	SDL_Surface* message = TTF_RenderText_Blended( font, text, sdl_color );

	switch (message->format->BytesPerPixel) {
		case 4:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				byte_order = GL_BGRA;
			else
				byte_order = GL_RGBA;
			break;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				byte_order = GL_BGR;
			else
				byte_order = GL_RGB;
			break;
	}

	// create new texture, with default filtering state (==mipmapping on)
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );

	// disable mipmapping on the new texture
	//... GL_LINEAR --> Smooth
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		message->w,
		message->h,
		0,
		byte_order,
		GL_UNSIGNED_BYTE,
		message->pixels
	);

	*result_w = message->w;
	*result_h = message->h;

	SDL_FreeSurface( message );
	return texture;
}


/*
 * SHOW IMAGE
 */

GLuint load_texture( char* file_name, int* result_w, int* result_h )
{
	GLuint texture;
	GLenum byte_order;

	SDL_Surface* image = IMG_Load( file_name );

	switch (image->format->BytesPerPixel) {
		case 4:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				byte_order = GL_BGRA;
			else
				byte_order = GL_RGBA;
			break;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				byte_order = GL_BGR;
			else
				byte_order = GL_RGB;
			break;
	}

	// create new texture, with default filtering state (==mipmapping on)
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );

	//...? disable mipmapping on the new texture
	//...? GL_LINEAR --> Smooth
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		image->w,
		image->h,
		0,
		byte_order,
		GL_UNSIGNED_BYTE,
		image->pixels
	);

	*result_w = image->w;
	*result_h = image->h;

	SDL_FreeSurface( image );
	return texture;
}


//EOF
