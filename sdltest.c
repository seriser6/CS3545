#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_opengl.h>
#include <stdio.h>
#include "vmath.h"

//New defines that we can use inside of mouse input handling to avoid magic numbers
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

//Means of the user exiting the main loop
static int user_exit = 0;

//Eventually these will be moved out of here into a "rendering" subsystem, which is why they are prefixed r_.
static void r_init();
static void r_drawFrame();

//Fancy new input declarations - we will also want to move this out of here.
static int keys_down[256];
static void input_update();
static void input_keyDown(SDLKey k);
static void input_keyUp(SDLKey k);
static void input_mouseMove(int xPos, int yPos);

/*
 * SDL_main
 * Program entry point
 */
int main(int argc, char* argv[])
{
	SDL_Event	event;
	SDL_Surface	*screen;

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_WM_SetCaption("Input Demo", "Input Demo");
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL);
	if(!screen)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

	//Renderer
	r_init();

	while(!user_exit)
	{
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
				exit(0);
			}
		}

		//Respond to keys being down
		input_update();

		//Draw the scene
		r_drawFrame();
	}

	SDL_Quit();
	return 0;
}

//We simply flag whether a key is up or down when it actually occurs, and then
//do the logic in a single update later on. If you don't do it this way things like
//using input to move the camera ends up being super stuttery, due to whatever reason.
static void input_keyDown(SDLKey k) { keys_down[k] = 1; if(k == SDLK_ESCAPE || k == SDLK_q) user_exit = 1; }
static void input_keyUp  (SDLKey k) { keys_down[k] = 0; }

//Mouse input is more complicated...
//The input event gives you where the mouse is located in screen coordinates.
//In our case, this means xPos will be between 0 and 1024, and yPos between
//0 and 768. Try printing out the values to get an idea of how the coordinates
//are oriented.
//Imagine how a first person camera typically works, and try to think of what is
//occurring in terms of matrix transformations. Since we know that the camera is
//looking down the -Z axis in the default OpenGL setup, and that we typically
//deal with rotations about axes, which rotations correspond to moving the mouse
//up/down or left/right?
//One other problem you will face is how to transform the screen coordinates of
//the mouse movement into a useful form to use inside of a rotation matrix.
//A simple way to achieve this is to reset the cursor to the middle of the window
//after each mouse movement. Then, you can get a delta movement out of the screen
//coordinates by simply subtracting half of the screen width/height. This can
//be translated into an angle of rotation directly, although you may need to scale
//the value (i.e. adjust the sensitivity).
static void input_mouseMove(int xPos, int yPos)
{
	float halfWinWidth, halfWinHeight,
		  dx, dy;

	halfWinWidth  = (float)WINDOW_WIDTH  / 2.0;
	halfWinHeight = (float)WINDOW_HEIGHT / 2.0;

	dx = xPos - halfWinWidth; dy = yPos - halfWinHeight;

	if (dx>0) glTranslatef(.02,0,0);
	if (dx<0) glTranslatef(-.02,0,0);
	if (dy>0) glTranslatef(0,.02,0);
	if (dy<0) glTranslatef(0,-.02,0);

	//Reset cursor to center
	SDL_WarpMouse(halfWinWidth, halfWinHeight);
}

//This function is currently being called each time we run through the main loop.
//By the time it is executed, all keys will have been registered as down or up.
//You can use this information inside of a simple check to do things like
//move the camera in a certain direction.
static void input_update()
{
	if(keys_down['w'])
		glRotatef(.5,0,1,0);
	if(keys_down['s'])
		glRotatef(-.5,0,1,0);
	if(keys_down['a'])
		glRotatef(-.5,1,0,0);
	if(keys_down['d'])
		glRotatef(.5,1,0,0);
//	if(keys_down['w'])
//		glTranslatef(0,0.01,0);
//	if(keys_down['s'])
//		glTranslatef(0,-0.01,0);
//	if(keys_down['a'])
//		glTranslatef(-0.01,0,0);
//	if(keys_down['d'])
//		glTranslatef(0.01,0,0);
	if(keys_down['='])
		glTranslatef(0,0,0.01);
	if(keys_down['-'])
		glTranslatef(0,0,-0.01);
}

/*
 * r_init
 * Initializes rendering state.
 */
static void r_init()
{
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.33, 1.0, 10.0);
	gluLookAt(
			1,1,5,
			0,0,0,
			0,1,0);
}

/*
 * r_drawFrame
 * Produces a final image of the scene.
 */
static void r_drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float x1 = -0.6;
	float y1 = -0.6;
	float z1 = -0.6;
	float x2 = 0.6;
	float y2 = 0.6;
	float z2 = 0.6;

	glColor3f(0.0, 1.0, 0.0); // green
	glBegin(GL_QUADS); // left face
		glVertex3f(x1, y1, z1);
		glVertex3f(x1, y1, z2);
		glVertex3f(x1, y2, z2);
		glVertex3f(x1, y2, z1);
	glEnd();

	glColor3f(0.0, 0.0, 1.0); // blue
	glBegin(GL_QUADS); // right face
		glVertex3f(x2, y1, z1);
		glVertex3f(x2, y1, z2);
		glVertex3f(x2, y2, z2);
		glVertex3f(x2, y2, z1);
	glEnd();

	glColor3f(1.0, 0.0, 0.0); // red
	glBegin(GL_QUADS); // bottom face
		glVertex3f(x1, y1, z1);
		glVertex3f(x1, y1, z2);
		glVertex3f(x2, y1, z2);
		glVertex3f(x2, y1, z1);
	glEnd();

	glColor3f(1.0, 0.5, 0.0); // orange
	glBegin(GL_QUADS); // top face
		glVertex3f(x1, y2, z1);
		glVertex3f(x1, y2, z2);
		glVertex3f(x2, y2, z2);
		glVertex3f(x2, y2, z1);
	glEnd();

	glColor3f(1.0, 1.0, 0.0); // yellow
	glBegin(GL_QUADS); // back face
		glVertex3f(x1, y1, z1);
		glVertex3f(x1, y2, z1);
		glVertex3f(x2, y2, z1);
		glVertex3f(x2, y1, z1);
	glEnd();

	glColor3f(1.0, 1.0, 1.0); // white
	glBegin(GL_QUADS); // front face
		glVertex3f(x1, y1, z2);
		glVertex3f(x1, y2, z2);
		glVertex3f(x2, y2, z2);
		glVertex3f(x2, y1, z2);
	glEnd();

	float x3 = -0.2;
	float y3 = -0.2;
	float z3 = -0.2;
	float x4 = 0.2;
	float y4 = 0.2;
	float z4 = 0.2;
	x1-=.02;
	y1-=.02;
	z1-=.02;
	x2+=.02;
	y2+=.02;
	z2+=.02;
	glColor3f(0.0, 0.0, 0.0); // black
	glBegin(GL_QUADS);
		glVertex3f(x3, y1, z1);
		glVertex3f(x3, y1, z2);
		glVertex3f(x3, y2, z2);
		glVertex3f(x3, y2, z1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(x4, y1, z1);
		glVertex3f(x4, y1, z2);
		glVertex3f(x4, y2, z2);
		glVertex3f(x4, y2, z1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(x1, y3, z1);
		glVertex3f(x1, y3, z2);
		glVertex3f(x2, y3, z2);
		glVertex3f(x2, y3, z1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(x1, y4, z1);
		glVertex3f(x1, y4, z2);
		glVertex3f(x2, y4, z2);
		glVertex3f(x2, y4, z1);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(x1, y1, z3);
		glVertex3f(x1, y2, z3);
		glVertex3f(x2, y2, z3);
		glVertex3f(x2, y1, z3);
	glEnd();
	glBegin(GL_QUADS);
		glVertex3f(x1, y1, z4);
		glVertex3f(x1, y2, z4);
		glVertex3f(x2, y2, z4);
		glVertex3f(x2, y1, z4);
	glEnd();

	SDL_GL_SwapBuffers();
}
