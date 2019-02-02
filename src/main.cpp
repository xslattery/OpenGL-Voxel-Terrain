//
//  main.cpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

// Library Headders:
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE // FIXES : gl.h and gl3.h conflict.
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Std Headders:
#include <map>
#include <vector>
#include <string>

// My Headders:
// #include "osx_fileio.h"
#include "typedefs.hpp"
#include "shader.hpp"
#include "text.hpp"
#include "sprite.hpp"
#include "perlin.hpp"
#include "chunk.hpp"

// Preprocessor macros:
#define DEBUG 					1
#define W_CENTER_ON_DISPLAY		1
#define GL_LOG_VERSION			0

struct Window {
	GLFWwindow *handle;
	int width, height;
	int vWidth, vHeight;
	bool vsync;
	bool fullscreen;
	bool resizable;
};

struct Application {
	FT_Library freeType;

	bool running;
	float deltaTime;
	glm::vec2 mousePosition;

	Window window;
};

struct Camera2D {
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 position;
	float scale;
	glm::vec2 aspect;
};

struct Camera3D {
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	float rotationAroundY;
	float rotationAroundX;

	float speed = 32;

	float fov;
	glm::vec2 aspect;
};

struct Skybox {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int vbo_color = 0;
	unsigned int ebo = 0;

	unsigned int numIndices = 0;

	glm::vec3 position = glm::vec3(0);
	glm::mat4 matrix = glm::mat4(1);
};

void createSkybox( Skybox *c, glm::vec3 pos, unsigned int shaderID );
void renderSkybox( Skybox *c, unsigned int shaderID );



void Run();
void Exit();
void InitState();
void Update();
void Render();
void HandleInput();

void glfwCallback_resize( GLFWwindow* window );

void ResizeProjectionMatrices();
void ResizeWindow( int width, int height );
void CreateWindow();



/////////////////////////////////////////////
//
// VARIABLES:
//

Application app;
Camera2D uiCamera;
Camera3D gameCamera;

struct {
	unsigned int uiTextShader;
	unsigned int uiTextTexture;
	PackedSpriteMap packedSpriteMap;
	Text uiDebugFPS;
	Text uiDebugStats;
	ColoredSpriteBatch testColoredSpriteBatch;
	Textured_Sprite texturedSprite;
} robj;

std::vector<glm::vec4> rectPositions;
std::vector<glm::vec4> rectColors;

float pastDeltas[30];
float deltaAverage;
float loopTime;

unsigned int skyboxShader;
Skybox skybox;

bool cursorHidden = false;

#define WORLD_X 8
#define WORLD_Z 8
#define WORLD_Y 2
unsigned int chunkShader;
Chunk* chunk[WORLD_X][WORLD_Z][WORLD_Y];
bool chunksGenerated = false;

unsigned long long vertexDataAmmount = 0;

//////////////////////////////////////////////////////////////////
//
// ENTRY POINT:
//

int main () { Run(); }

void Run () {

	//
	// Init libraries:
	if ( FT_Init_FreeType( &app.freeType ) ) { ERROR( "Failed to Init FreeType.\n"); return; }
	if ( !glfwInit() ) { ERROR( "Failed to Init glfw.\n" ); FT_Done_FreeType( app.freeType ); return; }

	app.window.width = 1280;
	app.window.height = 800;
	app.window.vsync = true;
	app.window.resizable = true;
	app.window.fullscreen = false;

	CreateWindow();
	InitState();

	double deltaTimeCalculator = glfwGetTime();
	app.running = true;

	while ( app.running ) {
		app.deltaTime = (float)(glfwGetTime() - deltaTimeCalculator);
		deltaTimeCalculator = glfwGetTime();

		HandleInput();
		Update();
		Render();

		loopTime = glfwGetTime() - deltaTimeCalculator;

		glfwSwapBuffers( app.window.handle );
	}

	Exit();
}

void Exit () {
	FT_Done_FreeType( app.freeType );
	glfwTerminate();
}







////////////////////////////////////////////////////////////////////////////////////////////
//
// CALLBACK FUNCTIONS:
//

void glfwCallback_resize( GLFWwindow* window ) {

	int tmp_w, tmp_h; glfwGetWindowSize( window, &tmp_w, &tmp_h );
	if( tmp_w != app.window.width || tmp_h != app.window.height ) {	
		app.window.width = tmp_w; app.window.height = tmp_h;
		glfwGetFramebufferSize( window, &app.window.vWidth, &app.window.vHeight );
		glViewport( 0, 0, app.window.vWidth, app.window.vHeight );
	}

	ResizeProjectionMatrices();

	Update();
	Render();
}

void glfwCallback_focus ( GLFWwindow* window, int state ) {
	if ( state == GLFW_TRUE ) { // Gained input focus
		cursorHidden = true;
	} else if ( state == GLFW_FALSE ) { // Lost input focus
		cursorHidden = false;
	}
}









/////////////////////////////////////////////////////////////////////////////////////////
//
// HELPER FUNCTIONS:
//

void ResizeProjectionMatrices() {
	uiCamera.aspect = glm::vec2( (float)app.window.vWidth, (float)app.window.vHeight );
	uiCamera.projectionMatrix = glm::ortho( 0.0f, uiCamera.aspect.x, uiCamera.aspect.y, 0.0f, 0.1f, 100.0f);
	gameCamera.aspect = glm::vec2( (float)app.window.width/app.window.height*10, (float)app.window.width/app.window.height*app.window.height/app.window.width*10 );
	gameCamera.projectionMatrix = glm::perspective( gameCamera.fov/180.0f*(float)M_PI, gameCamera.aspect.x/gameCamera.aspect.y, 0.1f, 1000.0f );
}

void ResizeWindow ( int width, int height ) {
	app.window.width = width;
	app.window.height = height;
	CreateWindow();
	ResizeProjectionMatrices();
}

void CreateWindow () {

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR,     3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR,     3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE,            GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT,     GL_TRUE );
	glfwWindowHint( GLFW_RESIZABLE,                 app.window.resizable );
	glfwWindowHint( GLFW_COCOA_RETINA_FRAMEBUFFER,  GL_TRUE );

	//
	// Create the window:
	if ( app.window.handle == 0 ) {
		app.window.handle = glfwCreateWindow( app.window.width, app.window.height, "Application", 0, 0 );
	}
	else {
		GLFWwindow *newWindow = glfwCreateWindow( app.window.width, app.window.height, "Application", 0, app.window.handle );
		glfwDestroyWindow( app.window.handle );
		app.window.handle = newWindow;
	}

	if ( !app.window.handle ) {		
		ERROR( "Failed to create window.\n" );
		Exit();
		return;
	} else {
		glfwMakeContextCurrent( app.window.handle );
		glfwGetWindowSize( app.window.handle, &app.window.width, &app.window.height );
		glfwGetFramebufferSize( app.window.handle, &app.window.vWidth, &app.window.vHeight ); 

		if ( W_CENTER_ON_DISPLAY ) {
			const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
			unsigned int monitor_width = mode->width;
			unsigned int monitor_height = mode->height;
			glfwSetWindowPos( app.window.handle, monitor_width/2 - app.window.width/2, monitor_height/2 - app.window.height/2);
		}

		glfwSwapInterval( app.window.vsync ); // Enable/Disable Vsync
		glfwSetWindowRefreshCallback( app.window.handle, glfwCallback_resize);
		glfwSetWindowFocusCallback( app.window.handle, glfwCallback_focus );
	}

	//
	// OpenGL Setup:
	glViewport( 0, 0, app.window.vWidth, app.window.vHeight );
	glEnable( GL_DEPTH_TEST );
	glCullFace( GL_BACK );	
	glEnable( GL_CULL_FACE );
	glEnable( GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	// glClearColor ( 0.1f, 0.1f, 0.1f, 1.0 );
	glClearColor ( 0/255.0f, 173/255.0f, 205/255.0f, 1.0f );
	

	#if GL_LOG_VERSION
		LOG( "OpenGL loaded\n" );
		LOG( "Vendor:   " << glGetString(GL_VENDOR) << "\n" );
		LOG( "Renderer: " << glGetString(GL_RENDERER) << "\n" );
		LOG( "Version:  " << glGetString(GL_VERSION) << "\n" );
	#endif

}

void createSkybox ( Skybox *c, glm::vec3 pos, unsigned int shaderID ) {

	c->position = pos;
	c->matrix = glm::translate( glm::mat4(1), pos );

	// ============================================================

	std::vector<GLfloat> vertices;
	std::vector<unsigned char> vertex_colors;
	std::vector<unsigned int> indices;

	GLfloat tmp_vert_array[ 24 ] = { 
		1, 1, 1,
		-1, 1, 1,
		1, -1, 1,
		-1, -1, 1,

		1, 1, -1,
		-1, 1, -1,
		1, -1, -1,
		-1, -1, -1,
	};
	vertices.insert( vertices.end(), tmp_vert_array, tmp_vert_array + 24 );

	// 100, 214, 214
	// 80, 181, 181
	// 26, 126, 188

	unsigned char tmp_color_array[ 32 ] = { 
		100,	214,	214,	255,
		100,	214,	214,	255,
		26,		126,	188,	255,
		26, 	126,	188,	255,

		100,	214,	214,	255,
		100,	214,	214,	255,
		26,		126,	188,	255,
		26, 	126,	188,	255,
	};
	vertex_colors.insert( vertex_colors.end(), tmp_color_array, tmp_color_array + 32 );

	unsigned int tmp_indices [36] = {
		0, 1, 2, // Z-NEG
		1, 3, 2,

		4, 0, 6, // X-NEG
		0, 2, 6,

		4, 5, 0, // Y-NEG
		5, 1, 0,

		5, 4, 7, // Z-POS
		4, 6, 7,

		1, 5, 3, // X-POS
		5, 7, 3,

		2, 3, 6, // Y-POS
		3, 7, 6,
	};
	c->numIndices += 36;
	indices.insert( indices.end(), tmp_indices, tmp_indices + 36 );

	// --------------------------------

	if ( c->vao == 0 ) glGenVertexArrays( 1, &c->vao );
	if ( c->vbo == 0 ) glGenBuffers( 1, &c->vbo );
	if ( c->vbo_color == 0 ) glGenBuffers( 1, &c->vbo_color );
	if ( c->ebo == 0 ) glGenBuffers( 1, &c->ebo );

	glBindVertexArray( c->vao );

		glBindBuffer( GL_ARRAY_BUFFER, c->vbo );
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW );

			unsigned int posAttrib = glGetAttribLocation( shaderID, "position" );
			glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( posAttrib );

		glBindBuffer( GL_ARRAY_BUFFER, c->vbo_color );
			glBufferData( GL_ARRAY_BUFFER, vertex_colors.size() * sizeof(unsigned char), vertex_colors.data(), GL_DYNAMIC_DRAW );

			unsigned int colorAttrib = glGetAttribLocation( shaderID, "color" );
			glVertexAttribPointer( colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)0 );
			glEnableVertexAttribArray( colorAttrib );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, c->ebo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW );
	
	glBindVertexArray( 0 );

}

void renderSkybox ( Skybox *c, unsigned int shaderID ) {
	setUniformMat4( shaderID, "model", c->matrix );
	glBindVertexArray( c->vao );
	glDrawElements( GL_TRIANGLES, c->numIndices, GL_UNSIGNED_INT, 0 );
}







////////////////////////////////////////////////////////////////////////////////
//
// MAIN LOGIC FUNCTIONS:
//

//
// INPUT:
void HandleInput () {
	glfwPollEvents();

	if ( glfwWindowShouldClose(app.window.handle) ) app.running = false;

	static bool escapePressed = false;
	if ( glfwGetKey(app.window.handle, GLFW_KEY_ESCAPE) ) {
		if ( escapePressed == false ) {
			if ( cursorHidden == true ) { cursorHidden = false; }
			else if ( cursorHidden == false ) { cursorHidden = true; }
		}
		escapePressed = true;
	} else { escapePressed = false; }

	if ( cursorHidden == false ) {
		glfwSetInputMode( app.window.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		if ( glfwGetMouseButton(app.window.handle, GLFW_MOUSE_BUTTON_LEFT) ) { cursorHidden = true; }
	}

	if ( cursorHidden == true ) {
		glfwSetInputMode( app.window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

		double mouseCursor_xpos, mouseCursor_ypos;
		glfwGetCursorPos( app.window.handle, &mouseCursor_xpos, &mouseCursor_ypos );
		gameCamera.rotationAroundY += ((float)mouseCursor_xpos - app.mousePosition.x) * app.deltaTime * 0.5f;
		gameCamera.rotationAroundX += ((float)mouseCursor_ypos - app.mousePosition.y) * app.deltaTime * 0.5f;

		gameCamera.forward = glm::vec3( -gameCamera.viewMatrix[0].z, -gameCamera.viewMatrix[1].z, -gameCamera.viewMatrix[2].z );
		gameCamera.right = glm::vec3( gameCamera.viewMatrix[0].x, gameCamera.viewMatrix[1].x, gameCamera.viewMatrix[2].x );

		if ( glfwGetKey( app.window.handle, GLFW_KEY_W ) ) { gameCamera.position += gameCamera.forward * app.deltaTime * gameCamera.speed; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_S ) ) { gameCamera.position -= gameCamera.forward * app.deltaTime * gameCamera.speed; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_D ) ) { gameCamera.position += gameCamera.right * app.deltaTime * gameCamera.speed; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_A ) ) { gameCamera.position -= gameCamera.right * app.deltaTime * gameCamera.speed; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_SPACE ) ) { gameCamera.position += glm::vec3(0, 1, 0) * app.deltaTime * gameCamera.speed; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_LEFT_SHIFT ) ) { gameCamera.position -= glm::vec3(0, 1, 0) * app.deltaTime * gameCamera.speed; }

		if ( glfwGetKey( app.window.handle, GLFW_KEY_PERIOD ) ) { gameCamera.rotationAroundY += app.deltaTime; }
		if ( glfwGetKey( app.window.handle, GLFW_KEY_COMMA ) ) { gameCamera.rotationAroundY -= app.deltaTime; }

		glm::mat4 translationMatrix = gameCamera.viewMatrix = glm::translate( glm::scale(glm::mat4(1), glm::vec3(1, 1, -1)), -gameCamera.position ); 
		glm::mat4 rotationMatrixY = glm::rotate( glm::mat4(1), gameCamera.rotationAroundY, glm::vec3(0,1,0) );
		glm::mat4 rotationMatrixX = glm::rotate( glm::mat4(1), gameCamera.rotationAroundX, glm::vec3(1,0,0) );
		gameCamera.viewMatrix =  rotationMatrixX * rotationMatrixY * translationMatrix;

		static bool gPressed = false;
		if ( glfwGetKey( app.window.handle, GLFW_KEY_G ) && !gPressed ) {
			gPressed = true;
			for ( int x = 0; x < WORLD_X; ++x ) {
				for ( int z = 0; z < WORLD_Z; ++z ) {
					for ( int y = 0; y < WORLD_Y; ++y ) {
						chunk[x][z][y] = new Chunk();
						createChunk ( chunk[x][z][y], glm::vec3(x*64 - 64*WORLD_X/2, y*64, z*64 - 64*WORLD_Z/2) );
						generateChunkMesh ( chunk[x][z][y], chunkShader );
					}
				}
			}
			chunksGenerated = true;
		}

	}

	if ( glfwGetKey( app.window.handle, GLFW_KEY_1 ) ) { ResizeWindow( 640, 480 ); }
	if ( glfwGetKey( app.window.handle, GLFW_KEY_2 ) ) { ResizeWindow( 1280, 800 ); }
	if ( glfwGetKey( app.window.handle, GLFW_KEY_3 ) ) { ResizeWindow( 1440, 900 ); }

	double temp_cursorPosX, temp_cursorPosY;
	glfwGetCursorPos( app.window.handle, &temp_cursorPosX, &temp_cursorPosY);
	app.mousePosition = glm::vec2( (float)temp_cursorPosX, (float)temp_cursorPosY );
}

//
// INIT:
void InitState () {

	uiCamera.position = glm::vec3( 0, 0, 50);
	uiCamera.scale = 1;
	uiCamera.aspect = glm::vec2( (float)app.window.vWidth, (float)app.window.vHeight );
	uiCamera.viewMatrix = glm::translate( glm::mat4(1), -uiCamera.position ); 

	gameCamera.position = glm::vec3( 0, 32, 0);
	gameCamera.fov = 75;
	gameCamera.aspect = glm::vec2( (float)app.window.width/app.window.height*10, (float)app.window.width/app.window.height*app.window.height/app.window.width*10 );
	gameCamera.viewMatrix = glm::translate( glm::scale(glm::mat4(1), glm::vec3(1, 1, -1)), -gameCamera.position ); 

	ResizeProjectionMatrices();

	robj.uiTextShader = LoadShaders( "res/shaders/textshader_vert.glsl", "res/shaders/textshader_frag.glsl" );
	robj.uiTextTexture = createPackedGlyphTexture( &robj.packedSpriteMap, app.freeType, 32, "res/SFMono-Regular.ttf" );

	robj.uiDebugFPS.setPosition( glm::vec3(10, 10, 0) );
	robj.uiDebugFPS.create_text_mesh( &robj.packedSpriteMap, robj.uiTextShader, "Hello\nWorld!", 0, TOP_LEFT | MONOSPACED);

	robj.uiDebugStats.setPosition( glm::vec3(app.window.vWidth-10, 10, 0) );
	robj.uiDebugStats.create_text_mesh( &robj.packedSpriteMap, robj.uiTextShader, "Hello\nWorld!", 0, TOP_RIGHT | MONOSPACED);

	robj.testColoredSpriteBatch.shaderID = LoadShaders( "res/shaders/spritebatchshader_color_vert.glsl", "res/shaders/spritebatchshader_color_frag.glsl" );

	rectPositions.reserve(100);
	rectColors.reserve(100);
	for (int i = 0; i < 100; ++i) {
		rectPositions.push_back( glm::vec4(rand()%100-50, rand()%100-50, rand()%5-2.5f, rand()%5-2.5f) );
		rectColors.push_back( glm::vec4(rand()%255/255.0f, rand()%255/255.0f, rand()%255/255.0f, rand()%128/255.0f+0.5f) );
	}

	robj.texturedSprite = Textured_Sprite( glm::vec3(-3, 25, 20), glm::vec2(1), 0, glm::vec2(2, 2), glm::vec2(0.0f) );
	robj.texturedSprite.transform.calculateMatrix();
	robj.texturedSprite.mesh.shaderID = LoadShaders( "res/shaders/spriteshader_textured_vert.glsl", "res/shaders/spriteshader_textured_frag.glsl" );
	LoadTexture( &robj.texturedSprite.mesh.texID, "res/TestImage.png" );
	generateTexturedSpriteMesh( &robj.texturedSprite );

	chunkShader = LoadShaders( "res/shaders/chunkShader_vert.glsl", "res/shaders/chunkShader_frag.glsl" );
	// createChunk ( &chunk, glm::vec3(-32, -32, -32) );
	// generateChunkMesh ( &chunk, chunkShader );

	skyboxShader = LoadShaders( "res/shaders/chunkShader_vert.glsl", "res/shaders/chunkShader_frag.glsl" );
	createSkybox( &skybox, glm::vec3(0, 20, 0), skyboxShader );
}


//
// UPDATE:
void Update () {

	static int dcount = 0;
	pastDeltas[dcount] = app.deltaTime;
	dcount++;
	if ( dcount >= 30 ) dcount = 0;
	deltaAverage = 0;
	for ( int i = 0; i < 30; ++i ) {
		deltaAverage += pastDeltas[i];
	}
	deltaAverage /= 30;

	robj.uiDebugFPS.create_text_mesh( &robj.packedSpriteMap, robj.uiTextShader, (
	"FPS: " + std::to_string(1.0f/deltaAverage) + "\n" +
	"DT: " + std::to_string(deltaAverage*1000) + "ms\n" +
	"FT:" + std::to_string(loopTime*1000) + "ms\n" +
	"\n\nPress 'G' to generate world...\n" +
	std::to_string(vertexDataAmmount)
	).c_str(),
	0, TOP_LEFT | MONOSPACED);

	robj.uiDebugStats.setPosition( glm::vec3(app.window.vWidth-10, 10, 0) );
	robj.uiDebugStats.create_text_mesh( &robj.packedSpriteMap, robj.uiTextShader, (
	"Aspect: " + std::to_string(gameCamera.aspect.x) + ", " + std::to_string(gameCamera.aspect.y) + "\n" +
	"WD: " + std::to_string(app.window.width) + ", " + std::to_string(app.window.height) + "\n" +
	"VD: " + std::to_string(app.window.vWidth) + ", " + std::to_string(app.window.vHeight) + "\n" +
	"MP: " + std::to_string((int)app.mousePosition.x) + ", " + std::to_string((int)app.mousePosition.y) + "\n\n" +
	"Rect Count = " + std::to_string(rectPositions.size())
	).c_str(),
	0, TOP_RIGHT | MONOSPACED);
	
	prepairColoredSpriteBatchForPush( &robj.testColoredSpriteBatch );
	for (int i = 0; i < rectPositions.size(); ++i) {
		rectPositions[i] = glm::vec4( rectPositions[i].x+rectPositions[i].z*app.deltaTime, rectPositions[i].y+rectPositions[i].w*app.deltaTime, rectPositions[i].z, rectPositions[i].w );
		pushToColoredSpriteBatch( &robj.testColoredSpriteBatch, glm::vec3(rectPositions[i].x, rectPositions[i].y, 100.0f), glm::vec2(1), 0, glm::vec2(1), glm::vec2(0.5), rectColors[i] );
	}
	buildColoredSpriteBatch( &robj.testColoredSpriteBatch );

	// chunkMesh.matrix = glm::translate( chunkMesh.matrix, -chunkMesh.position );
	// chunkMesh.matrix = glm::rotate( chunkMesh.matrix, app.deltaTime*0.1f, glm::vec3(0,1,0) );
	// chunkMesh.matrix = glm::translate( chunkMesh.matrix, chunkMesh.position );

	skybox.matrix = glm::translate( glm::mat4(1), gameCamera.position );
}


//
// RENDER:
void Render () {

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( skyboxShader );

		glDisable( GL_DEPTH_TEST );
		setUniformMat4( robj.testColoredSpriteBatch.shaderID, "view", gameCamera.viewMatrix );
		setUniformMat4( robj.testColoredSpriteBatch.shaderID, "projection", gameCamera.projectionMatrix );
		renderSkybox( &skybox, skyboxShader );
		glEnable( GL_DEPTH_TEST );

	glUseProgram( robj.testColoredSpriteBatch.shaderID );

		setUniformMat4( robj.testColoredSpriteBatch.shaderID, "view", gameCamera.viewMatrix );
		setUniformMat4( robj.testColoredSpriteBatch.shaderID, "projection", gameCamera.projectionMatrix );
		renderColoredSpriteBatch( &robj.testColoredSpriteBatch );

	glUseProgram( robj.texturedSprite.mesh.shaderID );

		setUniformMat4( robj.texturedSprite.mesh.shaderID, "view", gameCamera.viewMatrix );
		setUniformMat4( robj.texturedSprite.mesh.shaderID, "projection", gameCamera.projectionMatrix );
		renderTexturedSprite( &robj.texturedSprite );

	if ( chunksGenerated ) {
		glUseProgram( chunkShader );
		setUniformMat4( chunkShader, "view", gameCamera.viewMatrix );
		setUniformMat4( chunkShader, "projection", gameCamera.projectionMatrix );
		setUniform4f( chunkShader, "skyColor", glm::vec4(0/255.0f, 173/255.0f, 205/255.0f, 1.0f) );
		// renderChunk ( &chunk, chunkShader );
		for ( int x = 0; x < WORLD_X; ++x ) {
			for ( int z = 0; z < WORLD_Z; ++z ) {
				for ( int y = 0; y < WORLD_Y; ++y ) {
					renderChunk ( chunk[x][z][y], chunkShader );
				}
			}
		}
	}

	glUseProgram( robj.uiTextShader );

		glDisable( GL_DEPTH_TEST );
		setUniformMat4( robj.uiTextShader, "view", uiCamera.viewMatrix );
		setUniformMat4( robj.uiTextShader, "projection", uiCamera.projectionMatrix );
		setUniform4f( robj.uiTextShader, "overlayColor", glm::vec4(1.0f) );
		robj.uiDebugFPS.render( robj.uiTextShader, robj.uiTextTexture );
		robj.uiDebugStats.render( robj.uiTextShader, robj.uiTextTexture );
		glEnable( GL_DEPTH_TEST );
	
	glUseProgram( 0 );
}
