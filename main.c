/**
 * collision thing notes from clinton
 * 	for each (axis) {
 * 		projectPlayerBox()
 * 		projectTriangle()
 * 		findMinMaxOfThoseScalarValues()
 * 		compareProjections()
 * 	}
 */



/*
===========================================================================
File:		main.c
Author: 	Clinton Freeman
Created on:  	Feb 7, 2011
Description:	Texturing demo - you will need to change the path to the texture
		before this will work...
===========================================================================
*/

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_opengl.h>

#include "renderer_models.h"
#include "renderer_materials.h"
#include "common.h"
#include "vmath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static int user_exit = 0;
static float gravity = 0;
static int hit = 0;
static int textureStars;
static int textureBrick;
static int textureLava;
static int textureScore;
static double randx;
static double randz;
static double size;
static int points = 0;
static int won;
static int lost;

//INPUT DECLARATIONS

static void input_keyDown(SDLKey k);
static void input_keyUp(SDLKey k);
static void input_mouseMove(int dx, int dy);
static void input_update();

//CAMERA DECLARATIONS

typedef struct
{
	vec3_t	position;
	vec3_t	angles_deg;
	vec3_t	angles_rad;
} camera_t;

static camera_t camera;

static void camera_init();
static void camera_rotateX(float degree);
static void camera_rotateY(float degree);
static void camera_rotateZ(float degree);
static void camera_translateForward(float dist);
static void camera_translateStrafe(float dist);

//RENDERER DECLARATIONS

//NEW TEXTURE STUFF
//static void r_image_loadTGA(char *name, int *glTexID, int *width, int *height, int *bpp);

static void r_init();
static void r_setupProjection();
static void r_setupModelview();
static void r_drawFrame();

static const GLfloat flipMatrix[16] =
{1.0, 0.0,  0.0, 0.0,
 0.0, 0.0, -1.0, 0.0,
 0.0, 1.0,  0.0, 0.0,
 0.0, 0.0,  0.0, 1.0};

typedef struct
{
	int		glTexID;
	char	name[128];
	vec3_t	ambient, diffuse, specular;
	float	shine, shineStrength, transparency;
	int		width, height, bpp;
}
material_t;

static material_t materialList[MAX_TEXTURES];
static int stackPtr = 0;

void newgame() {

}
/*
 * SDL_main
 * Program entry point.
 */
int main(int argc, char* argv[])
{
	size = 32;
	srand(time(NULL));
	SDL_Event	event;
	SDL_Surface	*screen;

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_WM_SetCaption("Camera Demo", "Camera Demo");
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL);
	if(!screen)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

	r_init();

	float t = 0.0f;
	float dt = 0.1f;

	float currentTime = 0.0f;
	float accumulator = 0.0f;

	while(!user_exit)
	{
		if (won) {
			r_init();
		}
		float newTime = time(0);
		float deltaTime = newTime - currentTime;
		currentTime = newTime;

		if (deltaTime>0.25f)
			deltaTime = 0.25f;

		accumulator += deltaTime;

		while (accumulator>=dt)
		{
			accumulator -= dt;
			t += dt;
		}

		//Handle input
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				input_keyDown(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				input_keyUp(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				input_mouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_QUIT:
				user_exit = 1;
			}
		}

		input_update();

		r_drawFrame();
	}

	SDL_Quit();
	return 0;
}

/*
===========================================================================
	INPUT
===========================================================================
*/

static int keys_down[SDLK_LAST];

static void input_keyDown(SDLKey k) { keys_down[k] = 1; if(k == SDLK_ESCAPE || k == SDLK_ESCAPE) user_exit = 1; }
static void input_keyUp  (SDLKey k) { keys_down[k] = 0; }

/*
 * input_mouseMove
 */
void input_mouseMove(int dx, int dy)
{
	float halfWinWidth, halfWinHeight;

	halfWinWidth  = (float)WINDOW_WIDTH  / 2.0;
	halfWinHeight = (float)WINDOW_HEIGHT / 2.0;

	dx -= halfWinWidth; dy -= halfWinHeight;

	//Feed the deltas to the camera
	camera_rotateX(-dy/2.0);
	camera_rotateY(-dx/2.0);

	//Reset cursor to center
	SDL_WarpMouse(halfWinWidth, halfWinHeight);
}

/*
 * input_update
 */
static void input_update()
{
	//WASD
	//The input values are arbitrary
	if(keys_down[SDLK_w])
		camera_translateForward(0.05);
	if(keys_down[SDLK_s])
		camera_translateForward(-0.05);
	if(keys_down[SDLK_a])
		camera_translateStrafe(0.05);
	if(keys_down[SDLK_d])
		camera_translateStrafe(-0.05);

	//Reset, sometimes you can get pretty lost...
	if(keys_down[SDLK_r])
	{
		VectorClear(camera.angles_deg);
		VectorClear(camera.angles_rad);
		VectorClear(camera.position);
	}
}

/*
===========================================================================
	CAMERA
===========================================================================
*/

//Maintain a matrix for each rotation and one for translation
static float xRotMatrix[16], yRotMatrix[16], zRotMatrix[16], translateMatrix[16];

static void camera_init()
{
	camera.position[_X] = 0;
	camera.position[_Y] = 0;
	camera.position[_Z] = 0;

	MatrixIdentity(xRotMatrix);
	MatrixIdentity(yRotMatrix);
	MatrixIdentity(zRotMatrix);
	MatrixIdentity(translateMatrix);
}

//Rotations just increase/decrease the angle and compute a new radian value.
static void camera_rotateX(float degree)
{
	if(!((degree < 0 && camera.angles_deg[_X] < -70) || (degree > 0 && camera.angles_deg[_X] > 70))) {
		camera.angles_deg[_X] += degree;
		camera.angles_rad[_X] = camera.angles_deg[_X] * M_PI_DIV180;
	}
}

static void camera_rotateY(float degree)
{
		camera.angles_deg[_Y] += degree;
		camera.angles_rad[_Y] = camera.angles_deg[_Y] * M_PI_DIV180;
}

static void camera_rotateZ(float degree)
{
	camera.angles_deg[_Z] += degree;
	camera.angles_rad[_Z] = camera.angles_deg[_Z] * M_PI_DIV180;
}

static void camera_translateForward(float dist)
{
	float sinX, cosX, sinY, cosY, dx, dy, dz;

	sinY = sin(camera.angles_rad[_Y]);
	cosY = cos(camera.angles_rad[_Y]);

	sinX = sin(camera.angles_rad[_X]);
	cosX = cos(camera.angles_rad[_X]);

	//Free
//	dx =  -sinY * cosX * dist;
//	dy =  sinX * dist;
//	dz =  -cosY * cosX * dist;

	//Person
	dx =  -sinY * dist;
	dy =  0.0;
	dz =  -cosY * dist;

	camera.position[_X] += dx;
	camera.position[_Y] += dy;
	camera.position[_Z] += dz;
}

static void camera_translateStrafe(float dist)
{
	float sinX, cosX, sinY, cosY, dx, dy, dz, yPlus90;

	yPlus90 = (camera.angles_deg[_Y] + 90.0) * M_PI_DIV180;

	sinY = sin(yPlus90);
	cosY = cos(yPlus90);

	sinX = sin(camera.angles_rad[_X]);
	cosX = cos(camera.angles_rad[_X]);

	//Free
	dx =  -sinY * cosX * dist;
	dy =  0.0;
	dz =  -cosY * cosX * dist;

	//Person
	//dx =  -sinY * dist;
	//dy =  0.0;
	//dz =  -cosY * dist;

	camera.position[_X] += dx;
	camera.position[_Y] += dy;
	camera.position[_Z] += dz;
}

/*
===========================================================================
	TGA LOADING
===========================================================================
*/

#define HEADER_SIZE 18

//typedef unsigned char byte;

typedef struct
{
	unsigned char 	idLength, colormapType, imageType;
	unsigned char	colormapSize;
	unsigned short	colormapIndex, colormapLength;
	unsigned short	xOrigin, yOrigin, width, height;
	unsigned char	pixelSize, attributes;
}
tgaHeader_t;

/*
 * Function: renderer_img_loadTGA
 * Description: Loads a TARGA image file, uploads to GL, and returns the
 * texture ID. Only supports 24/32 bit.
 */
void renderer_img_loadTGA(char *name, int *glTexID, int *width, int *height, int *bpp)
{
	int				dataSize, rows, cols, i, j;
	GLuint			type;
	byte			*buf, *imageData, *pixelBuf, red, green, blue, alpha;

	FILE 			*file;
	tgaHeader_t		header;
	struct stat 	st;

	file = fopen(name, "rb");

	if(file == NULL)
	{
		printf("Loading TGA: %s, failed. Null file pointer.\n", name);
		return;
	}

	if(stat(name, &st))
	{
		printf("Loading TGA: %s, failed. Could not determine file size.\n", name);
		return;
	}

	if(st.st_size < HEADER_SIZE)
	{
		printf("Loading TGA: %s, failed. Header too short.\n", name);
		return;
	}

	buf = (byte *)malloc(st.st_size);
	fread(buf, sizeof(byte), st.st_size, file);

	fclose(file);

	memcpy(&header.idLength, 	 	&buf[0],  1);
	memcpy(&header.colormapType, 	&buf[1],  1);
	memcpy(&header.imageType, 		&buf[2],  1);
	memcpy(&header.colormapIndex, 	&buf[3],  2);
	memcpy(&header.colormapLength,  &buf[5],  2);
	memcpy(&header.colormapSize, 	&buf[7],  1);
	memcpy(&header.xOrigin,			&buf[8],  2);
	memcpy(&header.yOrigin,			&buf[10], 2);
	memcpy(&header.width,			&buf[12], 2);
	memcpy(&header.height,			&buf[14], 2);
	memcpy(&header.pixelSize,		&buf[16], 1);
	memcpy(&header.attributes,		&buf[17], 1);

	//Advance past the header
	buf += HEADER_SIZE;

	if(header.pixelSize != 24 && header.pixelSize != 32)
	{
		printf("Loading TGA: %s, failed. Only support 24/32 bit images.\n", name);
		return;
	}
	else if(header.pixelSize == 24)
		type = GL_RGB;
	else
		type = GL_RGBA;

	//Determine size of image data chunk in bytes
	dataSize = header.width * header.height * (header.pixelSize / 8);

	//Set up our texture
	*bpp 	 	= header.pixelSize;
	*width  	= header.width;
	*height 	= header.height;

	imageData = (byte *)malloc(dataSize);
	rows	  = *height;
	cols	  = *width;

	if(type == GL_RGB)
	{
		for(i = 0; i < rows; i++)
		{
			pixelBuf = imageData + (i * cols * 3);
			for(j = 0; j < cols; j++)
			{
				blue 	= *buf++;
				green 	= *buf++;
				red		= *buf++;

				*pixelBuf++ = red;
				*pixelBuf++ = green;
				*pixelBuf++ = blue;
			}
		}
	}
	else
	{
		for(i = 0; i < rows; i++)
		{
			pixelBuf = imageData + (i * cols * 4);
			for(j = 0; j < cols; j++)
			{
				blue 	= *buf++;
				green 	= *buf++;
				red		= *buf++;
				alpha	= *buf++;

				*pixelBuf++ = red;
				*pixelBuf++ = green;
				*pixelBuf++ = blue;
				*pixelBuf++ = alpha;
			}
		}
	}

	//Upload the texture to OpenGL
	glGenTextures(1, glTexID);
	glBindTexture(GL_TEXTURE_2D, *glTexID);

	//Default OpenGL settings have GL_TEXTURE_MAG/MIN_FILTER set to use
	//mipmaps... without these calls texturing will not work properly.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Upload image data to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, type, *width, *height,
			0, type, GL_UNSIGNED_BYTE, imageData);

	//Header debugging
	/*
	printf("Attributes: %d\n", 				header.attributes);
	printf("Colormap Index: %d\n", 			header.colormapIndex);
	printf("Colormap Length: %d\n", 		header.colormapLength);
	printf("Colormap Size: %d\n", 			header.colormapSize);
	printf("Colormap Type: %d\n", 			header.colormapType);
	printf("Height: %d\n", 					header.height);
	printf("Identification Length: %d\n",	header.idLength);
	printf("Image Type: %d\n", 				header.imageType);
	printf("Pixel Size: %d\n", 				header.pixelSize);
	printf("Width: %d\n", 					header.width);
	printf("X Origin: %d\n", 				header.xOrigin);
	printf("Y Origin: %d\n", 				header.yOrigin);
	*/
}

/*
===========================================================================
	RENDERER
===========================================================================
*/

/*
 * r_init
 * Perform any one-time GL state changes.
 */
static void r_init()
{
	lost = 0;
	won = 0;
	hit = 0;
	randx = rand()%21-10;
	randz = rand()%21-10;
	size = size/2;

	int myGLTexture, myTexWidth, myTexHeight, myTexBPP;

	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);

	//NEW TEXTURE STUFF
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//You might want to play with changing the modes
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

//	renderer_img_loadTGA("buttons.tga",
//			&textureButtons, &myTexWidth, &myTexHeight, &myTexBPP);
	renderer_img_loadTGA("brick.tga",
			&textureBrick, &myTexWidth, &myTexHeight, &myTexBPP);
	renderer_img_loadTGA("Starfield.tga",
			&textureStars, &myTexWidth, &myTexHeight, &myTexBPP);
	renderer_img_loadTGA("lava01.tga",
			&textureLava, &myTexWidth, &myTexHeight, &myTexBPP);
//	renderer_model_loadASE("submarine.ASE", efalse);
	renderer_model_loadASE("volcano.ASE", efalse);

	switch(points) {
	case 0:
		renderer_img_loadTGA("score0.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 1:
		renderer_img_loadTGA("score1.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 2:
		renderer_img_loadTGA("score2.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 3:
		renderer_img_loadTGA("score3.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 4:
		renderer_img_loadTGA("score4.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 5:
		renderer_img_loadTGA("score5.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 6:
		renderer_img_loadTGA("score6.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 7:
		renderer_img_loadTGA("score7.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 8:
		renderer_img_loadTGA("score8.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	case 9:
		renderer_img_loadTGA("score9.tga",
				&textureScore, &myTexWidth, &myTexHeight, &myTexBPP);
		break;
	}


	camera_init();

	r_setupProjection();
}

/*
 * r_setupProjection
 * Calculates the GL projection matrix. Only called once.
 */
static void r_setupProjection()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1.33, 0.1, 1024.0);
}

/*
 * r_setupModelview
 * Calculates the GL modelview matrix. Called each frame.
 */
static void r_setupModelview()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
 * r_setupModelview
 * Calculates the GL modelview matrix. Called each frame.
 */
static void r_setupModelviewRotate()
{
//	glMultMatrixf(flipMatrix);
	float sinX, cosX, sinY, cosY, sinZ, cosZ;

	sinX = sin(-camera.angles_rad[_X]);
	cosX = cos(-camera.angles_rad[_X]);

	xRotMatrix[5]  = cosX;
	xRotMatrix[6]  = sinX;
	xRotMatrix[9]  = -sinX;
	xRotMatrix[10] = cosX;

	sinY = sin(-camera.angles_rad[_Y]);
	cosY = cos(-camera.angles_rad[_Y]);

	zRotMatrix[0]  =  cosY;
	zRotMatrix[2]  = -sinY;
	zRotMatrix[8]  =  sinY;
	zRotMatrix[10] =  cosY;

	sinZ = sin(-camera.angles_rad[_Z]);
	cosZ = cos(-camera.angles_rad[_Z]);

	yRotMatrix[0] = cosZ;
	yRotMatrix[1] = sinZ;
	yRotMatrix[4] = -sinZ;
	yRotMatrix[5] = cosZ;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glMultMatrixf(flipMatrix);
	glMultMatrixf(xRotMatrix);
	glMultMatrixf(yRotMatrix);
	glMultMatrixf(zRotMatrix);
}

/*
 * r_setupModelview
 * Calculates the GL modelview matrix. Called each frame.
 */
static void r_setupModelviewTranslate()
{
	if (gravity==0 && (camera.position[_X]>2 || camera.position[_X]<-2 || camera.position[_Z]>2 || camera.position[_Z]<-2)) {
		if (!hit) {
			gravity = -0.05;
		}
	}
	if (camera.position[_Y]<-99 && !(camera.position[_X]>randx+size || camera.position[_X]<randx-size || camera.position[_Z]>randz+size || camera.position[_Z]<randz-size)) {
		if (!hit) {
			gravity = 0;
			points++;
			won = 1;
		}
	}
	if (camera.position[_Y]<-99) {
		hit = 1;
	}
	if (camera.position[_Y]<-199 && gravity) {
		lost = 1;
	}
	if (!lost) {
		camera.position[_Y] += gravity;
		translateMatrix[12] = -camera.position[_X];
		translateMatrix[13] = -camera.position[_Y];
		translateMatrix[14] = -camera.position[_Z];
	}
	glMatrixMode(GL_MODELVIEW);
//	glMultMatrixf(flipMatrix);
	glMultMatrixf(translateMatrix);
}

/*
 * r_drawFrame
 * Perform any drawing and setup necessary to produce a single frame.
 */
static void r_drawFrame()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	r_setupModelviewRotate();

	glClear(GL_DEPTH_BUFFER_BIT);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);

    glBindTexture(GL_TEXTURE_2D, textureStars);

    // Render the front quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  1.0f, -1.0f, -1.0f );
        glTexCoord2f(1, 0); glVertex3f( -1.0f, -1.0f, -1.0f );
        glTexCoord2f(1, 1); glVertex3f( -1.0f,  1.0f, -1.0f );
        glTexCoord2f(0, 1); glVertex3f(  1.0f,  1.0f, -1.0f );
    glEnd();

    // Render the left quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 0); glVertex3f(  1.0f, -1.0f, -1.0f );
        glTexCoord2f(1, 1); glVertex3f(  1.0f,  1.0f, -1.0f );
        glTexCoord2f(0, 1); glVertex3f(  1.0f,  1.0f,  1.0f );
    glEnd();

    // Render the back quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 0); glVertex3f(  1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 1); glVertex3f(  1.0f,  1.0f,  1.0f );
        glTexCoord2f(0, 1); glVertex3f( -1.0f,  1.0f,  1.0f );
    glEnd();

    // Render the right quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -1.0f, -1.0f, -1.0f );
        glTexCoord2f(1, 0); glVertex3f( -1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 1); glVertex3f( -1.0f,  1.0f,  1.0f );
        glTexCoord2f(0, 1); glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd();

    // Render the top quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -1.0f,  1.0f, -1.0f );
        glTexCoord2f(0, 0); glVertex3f( -1.0f,  1.0f,  1.0f );
        glTexCoord2f(1, 0); glVertex3f(  1.0f,  1.0f,  1.0f );
        glTexCoord2f(1, 1); glVertex3f(  1.0f,  1.0f, -1.0f );
    glEnd();

    // Render the bottom quad
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -1.0f, -1.0f, -1.0f );
        glTexCoord2f(0, 1); glVertex3f( -1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 1); glVertex3f(  1.0f, -1.0f,  1.0f );
        glTexCoord2f(1, 0); glVertex3f(  1.0f, -1.0f, -1.0f );
    glEnd();

	glClear(GL_DEPTH_BUFFER_BIT);

    r_setupModelviewTranslate();


	renderer_model_drawASE(0);

	glBindTexture(GL_TEXTURE_2D, textureLava);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(200,-201,200);
    glTexCoord2f(1,0); glVertex3f(200,-201,-200);
    glTexCoord2f(1,1); glVertex3f(-200,-201,-200);
    glTexCoord2f(0,1); glVertex3f(-200,-201,200);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textureScore);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(20,-200,20);
    glTexCoord2f(1,0); glVertex3f(20,-200,120);
    glTexCoord2f(1,1); glVertex3f(120,-200,120);
    glTexCoord2f(0,1); glVertex3f(120,-200,20);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, textureBrick);

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(2,-1,2);
    glTexCoord2f(1,0); glVertex3f(2,-1,-2);
    glTexCoord2f(1,1); glVertex3f(-2,-1,-2);
    glTexCoord2f(0,1); glVertex3f(-2,-1,2);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(randx+size,-100,randz+size);
    glTexCoord2f(size/4,0); glVertex3f(randx+size,-100,randz-size);
    glTexCoord2f(size/4,size/4); glVertex3f(randx-size,-100,randz-size);
    glTexCoord2f(0,size/4); glVertex3f(randx-size,-100,randz+size);
    glEnd();

//	DrawOverlay();
	r_setupModelview();

	glBindTexture(GL_TEXTURE_2D, textureBrick);
//	// Render a square in front of us
//	// Render the front quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.0f, SQRT_2*-0.1f, -0.2f );
//	glTexCoord2f(1, 0); glVertex3f(  0.05f, SQRT_2*-0.1f, -0.1f );
//	glTexCoord2f(1, 1); glVertex3f(  0.05f, SQRT_2*-0.05f, -0.1f );
//	glTexCoord2f(0, 1); glVertex3f(  0.0f, SQRT_2*-0.05f, -0.2f );
//	glEnd();
//	// Render the left quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.15f, SQRT_2*-0.1f, -0.3f );
//	glTexCoord2f(1, 0); glVertex3f(  0.0f, SQRT_2*-0.1f, -0.2f );
//	glTexCoord2f(1, 1); glVertex3f(  0.0f, SQRT_2*-0.05f, -0.2f );
//	glTexCoord2f(0, 1); glVertex3f(  0.15f, SQRT_2*-0.05f, -0.3f );
//	glEnd();
//	// Render the back quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.2f, SQRT_2*-0.1f, -0.2f );
//	glTexCoord2f(1, 0); glVertex3f(  0.15f, SQRT_2*-0.1f, -0.3f );
//	glTexCoord2f(1, 1); glVertex3f(  0.15f, SQRT_2*-0.05f, -0.3f );
//	glTexCoord2f(0, 1); glVertex3f(  0.2f, SQRT_2*-0.05f, -0.2f );
//	glEnd();
//	// Render the right quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.05f, SQRT_2*-0.1f, -0.1f );
//	glTexCoord2f(1, 0); glVertex3f(  0.2f, SQRT_2*-0.1f, -0.2f );
//	glTexCoord2f(1, 1); glVertex3f(  0.2f, SQRT_2*-0.05f, -0.2f );
//	glTexCoord2f(0, 1); glVertex3f(  0.05f, SQRT_2*-0.05f, -0.1f );
//	glEnd();
//	// Render the top quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.05f, SQRT_2*-0.05f, -0.1f );
//	glTexCoord2f(1, 0); glVertex3f(  0.2f, SQRT_2*-0.05f, -0.2f );
//	glTexCoord2f(1, 1); glVertex3f(  0.15f, SQRT_2*-0.05f, -0.3f );
//	glTexCoord2f(0, 1); glVertex3f(  0.0f, SQRT_2*-0.05f, -0.2f );
//	glEnd();
//	// Render the bottom quad
//	glBegin(GL_QUADS);
//	glTexCoord2f(0, 0); glVertex3f(  0.05f, SQRT_2*-0.1f, -0.1f );
//	glTexCoord2f(1, 0); glVertex3f(  0.2f, SQRT_2*-0.1f, -0.2f );
//	glTexCoord2f(1, 1); glVertex3f(  0.15f, SQRT_2*-0.1f, -0.3f );
//	glTexCoord2f(0, 1); glVertex3f(  0.0f, SQRT_2*-0.1f, -0.2f );
//	glEnd();


	SDL_GL_SwapBuffers();
}

/*
 * renderer_img_createMaterial
 */
int renderer_img_createMaterial(char *name, vec3_t ambient, vec3_t diffuse, vec3_t specular,
		float shine, float shineStrength, float transparency)
{
	material_t *currentMat = &materialList[stackPtr];

	currentMat->shine 			= shine;
	currentMat->shineStrength 	= shineStrength;
	currentMat->transparency 	= transparency;

	strcpy(currentMat->name, name);

	VectorCopy(ambient,  currentMat->ambient);
	VectorCopy(diffuse,  currentMat->diffuse);
	VectorCopy(specular, currentMat->specular);

	renderer_img_loadTGA(name, &(currentMat->glTexID),
			&(currentMat->width), &(currentMat->height), &(currentMat->bpp));
//	r_image_loadTGA(name, &(currentMat->glTexID),
//			&(currentMat->width), &(currentMat->height), &(currentMat->bpp));

	return stackPtr++;
}

int renderer_img_getMatGLID  (int i) { return materialList[i].glTexID; }
int renderer_img_getMatWidth (int i) { return materialList[i].width;   }
int renderer_img_getMatHeight(int i) { return materialList[i].height;  }
int renderer_img_getMatBpp   (int i) { return materialList[i].bpp;     }
