/*
 *  CSCI 441, Computer Graphics, Fall 2017
 *
 *  Project: Final Project
 *  File: main.cpp
 *
 *  Description:
 *      This file contains our final project game
 *
 *  Authors: Josh Nachtigal, Kyle Strayer, Savannah Paul
 *
 *
 */

//******************************************************************************

#include <GL/glew.h>
#include <GLFW/glfw3.h>			// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL/SOIL.h>		// for image loading

#include <stdio.h>				// for printf functionality
#include <stdlib.h>				// for exit functionality

#include <iostream>
#include <vector>					// for vector
#include <time.h>
#include <CSCI441/objects3.hpp>
#include <CSCI441/ShaderProgram3.hpp>
#include <CSCI441/TextureUtils.hpp>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "include/Marble.h"

//******************************************************************************
//
// Global Parameters

int windowWidth, windowHeight;
bool controlDown = false;
bool leftMouseDown = false;
bool viewOverlay = true;
int overlaySize = 150;
glm::vec2 mousePosition( -9999.0f, -9999.0f );

glm::vec3 cameraAngles( 1.82f, 2.01f, 25.0f );
glm::vec3 eyePoint(   10.0f, 10.0f, 10.0f );
glm::vec3 lookAtPoint( 0.0f,  0.0f,  0.0f );
glm::vec3 upVector(    0.0f,  1.0f,  0.0f );

GLuint platformVAOd;
GLuint platformTextureHandle;
GLuint finishTextureHandle;
GLuint mazeTextureHandle;

GLuint skyboxVAOds[6];						// all of our skybox VAOs
GLuint skyboxHandles[6];                    // all of our skybox handles

CSCI441::ShaderProgram* textureShaderProgram = NULL;
GLint uniform_modelMtx_loc, uniform_viewProjetionMtx_loc, uniform_tex_loc, uniform_color_loc;
GLint attrib_vPos_loc, attrib_vTextureCoord_loc;

CSCI441::ShaderProgram* shaderProgramHandle = NULL;
GLint mvp_uniform_location = -1, time_uniform_location = -1;
GLint vpos_attrib_location = -1;
glm::vec3 deadPos;

std::vector< Marble* > marbles;
Marble* user;
GLfloat groundSize = 30;
GLfloat marbleRadius = 1.0;
GLint numMarbles = 13;
glm::vec3 userPos;
glm::vec3 finishPos;
glm::vec3 userDir = glm::vec3(0, 0, 0);
glm::vec4 rotate;
int levelNum = 1;

double xAngle = 0;
double zAngle = 0;
vector<glm::vec3> mazePieces;
bool isWon = false;
bool isLost = false;

struct VertexTextured {
	float x, y, z;
	float s, t;
};

float backY = 0.0f;
float frontY = 0.0f;
GLfloat platformSize = groundSize + marbleRadius;

VertexTextured platformVertices[4] = {
	{ -platformSize, backY, -platformSize,   0.0f,  0.0f }, // 0 - BL
	{ platformSize, backY, -platformSize,   1.0f,  0.0f }, // 1 - BR
	{ -platformSize, frontY,  platformSize,   0.0f,  1.0f }, // 2 - TL
	{ platformSize, frontY,  platformSize,   1.0f,  1.0f }  // 3 - TR
};

//******************************************************************************
//
// Helper Functions

// convertSphericalToCartesian() ///////////////////////////////////////////////
//
// This function updates the camera's position in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraAngles is updated.
//
////////////////////////////////////////////////////////////////////////////////
void convertSphericalToCartesian() {
	eyePoint.x = cameraAngles.z * sinf( cameraAngles.x ) * sinf( cameraAngles.y );
	eyePoint.y = cameraAngles.z * -cosf( cameraAngles.y );
	eyePoint.z = cameraAngles.z * -cosf( cameraAngles.x ) * sinf( cameraAngles.y );
}

bool registerOpenGLTexture(unsigned char *textureData,
                           unsigned int texWidth, unsigned int texHeight,
                           GLuint &textureHandle) {
    if(textureData == 0) {
        fprintf(stderr,"Cannot register texture; no data specified.");
        return false;
    }

    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

    return true;
}

//******************************************************************************
//
// Event Callbacks

// error_callback() ////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
////////////////////////////////////////////////////////////////////////////////
static void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

// key_callback() //////////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's keypress callback.
//	Responds to key presses and key releases
//
////////////////////////////////////////////////////////////////////////////////

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	float maxH = .5;
	glm::vec3 plainDir = glm::vec3(1.0, 1.0, 1.0);

	if( (key == GLFW_KEY_ESCAPE || key == 'Q') && action == GLFW_PRESS )
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_LEFT:
			if (xAngle <= maxH) {
				xAngle += .1;
			}
			//rotate = glm::rotate(glm::mat4(), xAngle, glm::vec3(0.0, 0.0, 1.0))*glm::vec4(plainDir, 0);
			//userDir.z = rotate.z;
			//user->direction = userDir;
			break;
		case GLFW_KEY_RIGHT:
			if (xAngle >= -maxH) {
				xAngle -= .1;
			}
			//rotate = glm::rotate(glm::mat4(), -xAngle, glm::vec3(0.0, 0.0, 1.0))*glm::vec4(plainDir, 0);
			//userDir.z = rotate.z;	
			//user->direction = userDir;
			break;
		case GLFW_KEY_UP:
			if (zAngle <= maxH) {
				zAngle += .1;
			}

			//userDir.x = rotate.x;			
			//user->direction = userDir;
			break;
		case GLFW_KEY_DOWN:
			if (zAngle >= -maxH) {
				zAngle -= .1;
			}

			//userDir.x = rotate.x;			
			//user->direction = userDir;
			break;
		}
	}
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//		We will register this function as GLFW's mouse button callback.
//	Responds to mouse button presses and mouse button releases.  Keeps track if
//	the control key was pressed when a left mouse click occurs to allow
//	zooming of our arcball camera.
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		leftMouseDown = true;
		controlDown = (mods & GLFW_MOD_CONTROL);
	} else {
		leftMouseDown = false;
		mousePosition.x = -9999.0f;
		mousePosition.y = -9999.0f;
		controlDown = false;
	}
}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's cursor movement callback.
//	Responds to mouse movement.  When active motion is used with the left
//	mouse button an arcball camera model is followed.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	// make sure movement is in bounds of the window
	// glfw captures mouse movement on entire screen

	if( xpos > 0 && xpos < windowWidth ) {
		if( ypos > 0 && ypos < windowHeight ) {
			// active motion
			if( leftMouseDown ) {
				if( (mousePosition.x - -9999.0f) < 0.001f ) {
					mousePosition.x = xpos;
					mousePosition.y = ypos;
				} else {
					if( !controlDown ) {
						cameraAngles.x += (xpos - mousePosition.x)*0.005f;
						cameraAngles.y += (ypos - mousePosition.y)*0.005f;

						if( cameraAngles.y < 0 ) cameraAngles.y = 0.0f + 0.001f;
						if( cameraAngles.y >= M_PI ) cameraAngles.y = M_PI - 0.001f;
					} else {
						double totChgSq = (xpos - mousePosition.x) + (ypos - mousePosition.y);
						cameraAngles.z += totChgSq*0.01f;

						if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
						if( cameraAngles.z >= 60.0f ) cameraAngles.z = 60.0f;
					}
					convertSphericalToCartesian();

					mousePosition.x = xpos;
					mousePosition.y = ypos;
				}
			}
			// passive motion
			else {

			}
		}
	}
}

// scroll_callback() ///////////////////////////////////////////////////////////
//
//		We will register this function as GLFW's scroll wheel callback.
//	Responds to movement of the scroll where.  Allows zooming of the arcball
//	camera.
//
////////////////////////////////////////////////////////////////////////////////
static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset ) {
	double totChgSq = yOffset;
	cameraAngles.z += totChgSq*0.2f;

	if( cameraAngles.z <= 2.0f ) cameraAngles.z = 2.0f;
	if( cameraAngles.z >= 30.0f ) cameraAngles.z = 30.0f;

	convertSphericalToCartesian();
}

//******************************************************************************
//
// Setup Functions

// setupGLFW() /////////////////////////////////////////////////////////////////
//
//		Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
////////////////////////////////////////////////////////////////////////////////
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback(error_callback);

	// initialize GLFW
	if (!glfwInit()) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit(EXIT_FAILURE);
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );						// request forward compatible OpenGL context
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	// request OpenGL Core Profile context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );		// request OpenGL 3.x context
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );		// request OpenGL 3.3 context

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow(640, 480, "Graphics Final: Labyrinth", NULL, NULL);
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(	window );	// make the created window the current window
	glfwSwapInterval( 1 );				    // update our screen after at least 1 screen refresh

	glfwSetKeyCallback( 			  window, key_callback				  );	// set our keyboard callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function
	glfwSetCursorPosCallback(	  window, cursor_callback  			);	// set our cursor position callback function
	glfwSetScrollCallback(			window, scroll_callback			  );	// set our scroll wheel callback function

	return window;										// return the window that was created
}

// setupOpenGL() ///////////////////////////////////////////////////////////////
//
//      Used to setup everything OpenGL related.
//
////////////////////////////////////////////////////////////////////////////////
void setupOpenGL() {
	glEnable( GL_DEPTH_TEST );					// enable depth testing
	glDepthFunc( GL_LESS );							// use less than depth test

	glEnable(GL_BLEND);									// enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// use one minus blending equation

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black
}

// setupGLEW() /////////////////////////////////////////////////////////////////
//
//      Used to initialize GLEW
//
////////////////////////////////////////////////////////////////////////////////
void setupGLEW() {
	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	/* check for an error */
	if( glewResult != GLEW_OK ) {
		printf( "[ERROR]: Error initalizing GLEW\n");
		/* Problem: glewInit failed, something is seriously wrong. */
  	fprintf( stderr, "[ERROR]: %s\n", glewGetErrorString(glewResult) );
		exit(EXIT_FAILURE);
	} else {
		 fprintf( stdout, "[INFO]: GLEW initialized\n" );
		 fprintf( stdout, "[INFO]: Status: Using GLEW %s\n", glewGetString(GLEW_VERSION) );
	}

	if( !glewIsSupported( "GL_VERSION_2_0" ) ) {
		printf( "[ERROR]: OpenGL not version 2.0+.  GLSL not supported\n" );
		exit(EXIT_FAILURE);
	}
}

// setupTextures() /////////////////////////////////////////////////////////////
//
//      Load and register all the tetures for our program
//
////////////////////////////////////////////////////////////////////////////////
void setupTextures() {
	// and get handles for our full skybox
  printf( "[INFO]: registering skybox..." );
  fflush( stdout );
  if (levelNum == 1) {
	  platformTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/grassblades.png");
	  mazeTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/hedge.jpg");

	  skyboxHandles[0] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/back.png");   printf("."); fflush(stdout);
	  skyboxHandles[1] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/left.png");   printf("."); fflush(stdout);
	  skyboxHandles[2] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/front.png");   printf("."); fflush(stdout);
	  skyboxHandles[3] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/right.png");   printf("."); fflush(stdout);
	  skyboxHandles[4] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/bottom.png");		printf("."); fflush(stdout);
	  skyboxHandles[5] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox/top.png");   printf("."); fflush(stdout);
	  printf("skybox textures read in and registered!\n\n");

	  unsigned char *brickTexData;
	  int brickTexWidth, brickTexHeight;
	  CSCI441::TextureUtils::loadPPM("textures/brick.ppm", brickTexWidth, brickTexHeight, brickTexData);
	  registerOpenGLTexture(brickTexData, brickTexWidth, brickTexHeight, finishTextureHandle);
	  printf("[INFO]: brick texture read in and registered\n");
  }
  else if (levelNum == 2) {
	  platformTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/lavastone.png");
	  mazeTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/metal.jpg");

	  skyboxHandles[0] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/back.tga");   printf("."); fflush(stdout);
	  skyboxHandles[1] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/left.tga");   printf("."); fflush(stdout);
	  skyboxHandles[2] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/front.tga");   printf("."); fflush(stdout);
	  skyboxHandles[3] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/right.tga");   printf("."); fflush(stdout);
	  skyboxHandles[4] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/bottom.tga");		printf("."); fflush(stdout);
	  skyboxHandles[5] = CSCI441::TextureUtils::loadAndRegisterTexture("textures/skybox2/top.tga");   printf("."); fflush(stdout);
	  printf("skybox textures read in and registered!\n\n");

	  finishTextureHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/grassblades.png");
  }

}

void setupShaders() {
	textureShaderProgram = new CSCI441::ShaderProgram( "shaders/textureShader.v.glsl", "shaders/textureShader.f.glsl" );
	uniform_modelMtx_loc         = textureShaderProgram->getUniformLocation( "modelMtx" );
	uniform_viewProjetionMtx_loc = textureShaderProgram->getUniformLocation( "viewProjectionMtx" );
	uniform_tex_loc              = textureShaderProgram->getUniformLocation( "tex" );
	uniform_color_loc			 = textureShaderProgram->getUniformLocation( "color" );
	attrib_vPos_loc			     = textureShaderProgram->getAttributeLocation( "vPos" );
	attrib_vTextureCoord_loc 	 = textureShaderProgram->getAttributeLocation( "vTextureCoord" );

	//set up death shader program
	shaderProgramHandle = new CSCI441::ShaderProgram("shaders/customShader.v.glsl", "shaders/customShader.f.glsl");
	mvp_uniform_location = shaderProgramHandle->getUniformLocation("mvpMatrix");
	time_uniform_location = shaderProgramHandle->getUniformLocation("time");
	vpos_attrib_location = shaderProgramHandle->getAttributeLocation("vPosition");

}


// setupBuffers() //////////////////////////////////////////////////////////////
//
//      Create our VAOs & VBOs. Send vertex data to the GPU for future rendering
//
////////////////////////////////////////////////////////////////////////////////
void setupBuffers() {

	//////////////////////////////////////////
	//
	// PLATFORM

	GLfloat platformSize = groundSize + marbleRadius;

	unsigned short platformIndices[4] = { 0, 1, 2, 3 };

	glGenVertexArrays( 1, &platformVAOd );
	glBindVertexArray( platformVAOd );

	GLuint vbods[2];
	glGenBuffers( 2, vbods );

	glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

	glEnableVertexAttribArray( attrib_vPos_loc );
	glVertexAttribPointer( attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );

	glEnableVertexAttribArray( attrib_vTextureCoord_loc );
	glVertexAttribPointer( attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );


	//////////////////////////////////////////
	//
	// SKYBOX

	unsigned short groundIndices[4] = {
		0, 1, 2, 3
	};

	VertexTextured groundVertices[4] = {
			{ -40.0f, -40.0f, -40.0f,   0.0f,  0.0f }, // 0 - BL
			{  40.0f, -40.0f, -40.0f,   -1.0f,  0.0f }, // 1 - BR
			{ -40.0f, -40.0f,  40.0f,   0.0f,  -1.0f }, // 2 - TL
			{  40.0f, -40.0f,  40.0f,   -1.0f,  -1.0f }  // 3 - TR
	};

	VertexTextured leftWallVerts[4] = {
	  { -40.0f, -40.0f, -40.0f,   0.0f,  0.0f }, // 0 - BL
	  {  40.0f, -40.0f, -40.0f,   1.0f,  0.0f }, // 1 - BR
	  { -40.0f,  40.0f, -40.0f,   0.0f,  1.0f }, // 2 - TL
	  {  40.0f,  40.0f, -40.0f,   1.0f,  1.0f }  // 3 - TR
	};

	VertexTextured rightWallVerts[4] = {
	  { -40.0f, -40.0f,  40.0f,   0.0f,  0.0f }, // 0 - BL
	  {  40.0f, -40.0f,  40.0f,   -1.0f,  0.0f }, // 1 - BR
	  { -40.0f,  40.0f,  40.0f,   0.0f,  1.0f }, // 2 - TL
	  {  40.0f,  40.0f,  40.0f,   -1.0f,  1.0f }  // 3 - TR
	};

	VertexTextured backWallVerts[4] = {
	  { -40.0f, -40.0f, -40.0f,   0.0f,  0.0f }, // 0 - BL
	  { -40.0f, -40.0f,  40.0f,   -1.0f,  0.0f }, // 1 - BR
	  { -40.0f,  40.0f, -40.0f,   0.0f,  1.0f }, // 2 - TL
	  { -40.0f,  40.0f,  40.0f,   -1.0f,  1.0f }  // 3 - TR
	};

	VertexTextured frontWallVerts[4] = {
	  {  40.0f, -40.0f, -40.0f,   0.0f,  0.0f }, // 0 - BL
	  {  40.0f, -40.0f,  40.0f,   1.0f,  0.0f }, // 1 - BR
	  {  40.0f,  40.0f, -40.0f,   0.0f,  1.0f }, // 2 - TL
	  {  40.0f,  40.0f,  40.0f,   1.0f,  1.0f }  // 3 - TR
	};

	VertexTextured topWallVerts[4] = {
			{ -40.0f,  40.0f, -40.0f,   1.0f,  -1.0f }, // 0 - BL
			{  40.0f,  40.0f, -40.0f,   1.0f,  0.0f }, // 1 - BR
			{ -40.0f,  40.0f,  40.0f,   0.0f,  -1.0f }, // 2 - TL
			{  40.0f,  40.0f,  40.0f,   0.0f,  0.0f }  // 3 - TR
	};

	glGenVertexArrays( 6, skyboxVAOds );

  glBindVertexArray( skyboxVAOds[4] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

  glBindVertexArray( skyboxVAOds[3] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(leftWallVerts), leftWallVerts, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

  glBindVertexArray( skyboxVAOds[1] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(rightWallVerts), rightWallVerts, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

  glBindVertexArray( skyboxVAOds[0] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(backWallVerts), backWallVerts, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

  glBindVertexArray( skyboxVAOds[2] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(frontWallVerts), frontWallVerts, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

  glBindVertexArray( skyboxVAOds[5] );
  glGenBuffers(2, vbods);
  glBindBuffer( GL_ARRAY_BUFFER, vbods[0] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(topWallVerts), topWallVerts, GL_STATIC_DRAW );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbods[1] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW );
  glEnableVertexAttribArray(attrib_vPos_loc);
  glVertexAttribPointer(attrib_vPos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0);
  glEnableVertexAttribArray(attrib_vTextureCoord_loc);
  glVertexAttribPointer(attrib_vTextureCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(float) * 3));

}

void populateMarbles() {
    srand( time(NULL) );
    float rangeX = groundSize*2;
    float rangeZ = groundSize*2;
	for (int i = 0; i < numMarbles; i++) {
        // TODO: Populate our marble locations
        Marble* m = new Marble( glm::vec3( rand()/(float)RAND_MAX * rangeX - rangeX/2.0f,
                                   0.0f,
                                   (rangeZ * (i/(float)numMarbles)) - rangeZ/2.0f),
                            		glm::vec3( rand()/(float)RAND_MAX, 0.0, rand()/(float)RAND_MAX ),
                            		marbleRadius*(rand()/(float)RAND_MAX+0.25) );
        marbles.push_back( m );
    }
}

void populateMaze() {
	string line;
	ifstream ipf("config.txt");
	if (ipf.is_open()) {
		double xLoc = -groundSize/2;
		while (getline(ipf, line)) {
			double zLoc = -groundSize/2;
			for (int i = 0; i < line.size(); i++) {
				if (line[i] == 'O') {
					mazePieces.push_back(glm::vec3(-xLoc - groundSize / 20 , 2, zLoc + groundSize / 20));
				}
				if (line[i] == 'S') {
					userPos = glm::vec3(-xLoc, 1, zLoc);
				}
				if (line[i] == 'F') {
					finishPos = glm::vec3(-xLoc - groundSize / 10, .5, zLoc);
				}
				zLoc += groundSize/10;
			}
			xLoc += groundSize/10;
		}
	}
	user = new Marble(userPos, glm::vec3(1, 1, 1), 1.0);
	ipf.close();
}

//******************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// renderScene() ///////////////////////////////////////////////////////////////
//
//		This method will contain all of the objects to be drawn.
//
////////////////////////////////////////////////////////////////////////////////
void renderScene( glm::mat4 viewMatrix, glm::mat4 projectionMatrix ) {
	textureShaderProgram->useProgram();

	glm::mat4 m, vp = projectionMatrix * viewMatrix;
	glUniformMatrix4fv(uniform_modelMtx_loc, 1, GL_FALSE, &m[0][0]);
	glUniformMatrix4fv(uniform_viewProjetionMtx_loc, 1, GL_FALSE, &vp[0][0]);
	glUniform1ui(uniform_tex_loc, GL_TEXTURE0);

	glm::vec3 white(1,1,1);
	glUniform3fv( uniform_color_loc, 1, &white[0] );

	for( unsigned int i = 0; i < 6; i++ ) {
		glBindTexture( GL_TEXTURE_2D, skyboxHandles[i] );
		glBindVertexArray(skyboxVAOds[i]);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void*)0);
	}

	glBindTexture( GL_TEXTURE_2D, platformTextureHandle );
	//Rotate platform
	m = glm::rotate(m, float(xAngle), glm::vec3(1.0, 0.0, 0.0));
	m = glm::rotate(m, float(zAngle), glm::vec3(0.0, 0.0, 1.0));
	m = glm::scale(m, glm::vec3(groundSize, .5, groundSize));
	glUniformMatrix4fv(uniform_modelMtx_loc, 1, GL_FALSE, &m[0][0]);
	CSCI441::drawSolidCube(1);


	glBindTexture( GL_TEXTURE_2D, mazeTextureHandle );
	//Draw the obstacles/maze
	for( unsigned int i = 0; i < mazePieces.size(); i++ ) {
			m = glm::mat4();
			m = glm::rotate(m, float(xAngle), glm::vec3(1.0, 0.0, 0.0));
			m = glm::rotate(m, float(zAngle), glm::vec3(0.0, 0.0, 1.0));
			m = glm::translate(m, mazePieces[i]);
		//	m = glm::translate(m, glm::vec3(groundSize / 20, 0.0, groundSize / 20));
			m = glm::scale(m, glm::vec3(groundSize / 10, 3.0, groundSize / 10));

			glUniformMatrix4fv(uniform_modelMtx_loc, 1, GL_FALSE, &m[0][0]);
			CSCI441::drawSolidCube(1);
	}

	//Draw finish marker
	glBindTexture(GL_TEXTURE_2D, finishTextureHandle);
	m = glm::mat4(1.0);
	m = glm::rotate(m, float(xAngle), glm::vec3(1.0, 0.0, 0.0));
	m = glm::rotate(m, float(zAngle), glm::vec3(0.0, 0.0, 1.0));
	m = glm::translate(m, finishPos);
	m = glm::translate(m, glm::vec3(groundSize / 20, 0.0, groundSize / 20));
	m = glm::scale(m, glm::vec3(groundSize / 10, 0.0, groundSize / 10));
	glUniformMatrix4fv(uniform_modelMtx_loc, 1, GL_FALSE, &m[0][0]);
	CSCI441::drawSolidCube(1);

	//Draw the player
	m = glm::mat4(1.0);

	m = glm::rotate(m, float(xAngle), glm::vec3(1.0, 0.0, 0.0));
	m = glm::rotate(m, float(zAngle), glm::vec3(0.0, 0.0, 1.0));
	glm::vec3 loc = glm::vec3(user->location.x / groundSize, user->location.y / groundSize, user->location.z / groundSize);
	m = glm::translate(m, loc);
	glUniformMatrix4fv(uniform_modelMtx_loc, 1, GL_FALSE, &m[0][0]);

	if (isLost) {
		shaderProgramHandle->useProgram();
		//glEnableVertexAttribArray(vpos_attrib_location);
		//glVertexAttribPointer(vpos_attrib_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), &deadPos);

		glm::mat4 mvpMtx = projectionMatrix * viewMatrix * m;
		glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, &mvpMtx[0][0]);

		//glUniform3f(vpos_uniform_location, deadPos);
		glUniform1f(time_uniform_location, glfwGetTime());
		user->draw(mvpMtx, mvp_uniform_location, uniform_color_loc);
	}
	if (!isLost) {
		user->draw(m, uniform_modelMtx_loc, uniform_color_loc);
	}

}

void movePlayer() {
	double tol = 10 ^ -5;
	if (xAngle == 0 && zAngle == 0) {
		//don't move
	}
	else {
		user->moveForward(-zAngle, xAngle);
	}
}


void collideAndMove() {
	if (isLost) {
		//CSCI441::setVertexAttributeLocations(vpos_attrib_location);
		user->location.y -= .1;
		return;
	}

	float x = xAngle;
	float z = zAngle;
	for (int i = 0; i < mazePieces.size(); i++) {
		double distancex  = abs(user->location.x - zAngle - mazePieces.at(i).x);
		double distancez  = abs(user->location.z + xAngle - mazePieces.at(i).z);
		if ((distancex <= user->radius + groundSize / 20) && ((user->location.z <= mazePieces.at(i).z + groundSize / 20) && (user->location.z >= mazePieces.at(i).z - groundSize / 20))) {	//cube length from center to any side is groundLength / 20
			z = 0;
			//cout << "collidez" << endl;
		}
		if ((distancez <= user->radius + groundSize / 20) && ((user->location.x <= mazePieces.at(i).x + groundSize / 20) && (user->location.x >= mazePieces.at(i).x - groundSize / 20))) {	//cube length from center to any side is groundLength / 20
			x = 0;
			//debugging stuff
			//cout << "collidex" << endl << distancez << endl << user->radius + groundSize / 20 << endl;
			//cout << i << " " << mazePieces.at(i).x << " " << mazePieces.at(i).z << endl;
			//cout << " " << user->location.x - zAngle << " " << user->location.z + xAngle << endl;
			//cout << mazePieces.at(i).x + groundSize / 20 << " " << mazePieces.at(i).x - groundSize / 20 << endl;
			//cout << mazePieces.at(i).z + groundSize / 20 << " " << mazePieces.at(i).z - groundSize / 20 << endl;
		}
	}
	double distancex = abs(user->location.x - zAngle - finishPos.x);
	double distancez = abs(user->location.z + xAngle - finishPos.z);
	//finish point collision detection
	if ((distancex <= user->radius + groundSize / 20) && ((user->location.z <= finishPos.z + groundSize / 20) && (user->location.z >= finishPos.z - groundSize / 20))) {
		isWon = true;
	}
	if ((distancez <= user->radius + groundSize / 20) && ((user->location.x <= finishPos.x + groundSize / 20) && (user->location.x >= finishPos.x - groundSize / 20))) {
		isWon = true;
	}

	//edge collision
	if (user->location.x < -groundSize / 2 || user->location.z < -groundSize / 2
		|| user->location.x > groundSize / 2 || user->location.z > groundSize / 2) {
		isLost = true;
		deadPos = user->location;
	}
	user->moveForward(-z, x);
}

void startNewLevel() {
	setupTextures();
	populateMaze();
	isWon = false;
}


///*****************************************************************************
//
// Our main function

// main() ///////////////////////////////////////////////////////////////
//
//		Really you should know what this is by now.
//
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] ) {
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupGLEW();											// initialize all of the GLEW specific information
	setupShaders();										// load our shaders into memory
	setupBuffers();										// load all our VAOs and VBOs into memory
	setupTextures();									// load all textures into memory
	populateMaze();
	convertSphericalToCartesian();		// set up our camera position

	CSCI441::setVertexAttributeLocations( attrib_vPos_loc, -1, attrib_vTextureCoord_loc );
	CSCI441::drawSolidSphere( 1, 16, 16 );	// strange hack I need to make spheres draw - don't have time to investigate why..it's a bug with my library

	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
		glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, windowWidth, windowHeight );

		// set the projection matrix based on the window size
		// use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projectionMatrix = glm::perspective( 45.0f, windowWidth / (float) windowHeight, 0.001f, 100.0f );

		// set up our look at matrix to position our camera
		glm::mat4 viewMatrix = glm::lookAt( eyePoint,lookAtPoint, upVector );

		// draw everything to the window
		// pass our view and projection matrices as well as deltaTime between frames

		//move stuff

		collideAndMove();
		//cout << user->location.x << " " << user->location.z << endl;		//outputs current user location for debugging
		renderScene( viewMatrix, projectionMatrix );


		if (viewOverlay) {
			int overlayX = windowWidth - overlaySize;
			int overlayY = windowHeight - overlaySize;

			glEnable(GL_SCISSOR_TEST);
			glScissor(overlayX, overlayY, overlaySize, overlaySize);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glDisable(GL_SCISSOR_TEST);

			glViewport(overlayX, overlayY, overlaySize, overlaySize);

			// First person camera view matrix
			glm::mat4 viewMtx = glm::lookAt(glm::vec3(0, 30, 0), lookAtPoint, glm::vec3(-1, 0, 0));
			renderScene(viewMtx, projectionMatrix);
		}


		if (isWon) {
			levelNum++;
			startNewLevel();
		}

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen

		setupBuffers();
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
