/* Compilation on Linux: 
 g++ -std=c++17 ./src/*.cpp -o prog -I ./include/ -I./../common/thirdparty/ -lSDL2 -ldl
*/

// Third Party Libraries
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

// Our libraries
#include "Camera.hpp"
#include "Texture.hpp"
#include "gravity.hpp"
#include "World.hpp"

// vvvvvvvvvvvvvvvvvvvvvvvvvv Globals vvvvvvvvvvvvvvvvvvvvvvvvvv
// Globals generally are prefixed with 'g' in this application.

// Screen Dimensions
int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext = nullptr;
// Main loop flag
bool gQuit = false; // If this is quit = 'true' then the program terminates.
// Camera
Camera gCamera;

/**
* Initialization of the graphics application. Typically this will involve setting up a window
* and the OpenGL Context (with the appropriate version)
*
* @return void
*/
void InitializeProgram(){
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0){
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}
	
	// Setup the OpenGL Context
	// Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow(
    "Tesselation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

	// Check if Window did not create.
	if( gGraphicsApplicationWindow == nullptr ){
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Create an OpenGL Graphics Context
	gOpenGLContext = SDL_GL_CreateContext( gGraphicsApplicationWindow );
	if( gOpenGLContext == nullptr){
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Initialize GLAD Library
	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
		std::cout << "glad did not initialize" << std::endl;
		exit(1);
	}
}

/**
* Function called in the Main application loop to handle user input
*
* @return void
*/
void Input(EntityGod &entityGod){
    // Two static variables to hold the mouse position
    static int mouseX=gScreenWidth/2;
    static int mouseY=gScreenHeight/2; 

	// Event handler that handles various events in SDL
	// that are related to input and output
	SDL_Event e;
	//Handle events on queue
	while(SDL_PollEvent( &e ) != 0){
		// If users posts an event to quit
		// An example is hitting the "x" in the corner of the window.
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
		}
    if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
      std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
      gQuit = true;
    }
    if(e.type==SDL_MOUSEMOTION){
      // Capture the change in the mouse position
      mouseX+=e.motion.xrel;
      mouseY+=e.motion.yrel;
      gCamera.MouseLook(mouseX,mouseY);
    }
	}

  // Retrieve keyboard state
  const Uint8 *state = SDL_GetKeyboardState(NULL);

  // Camera
  // Update our position of the camera
  float acceleration = 0.008;
  glm::vec3 forwardStepDirection = glm::normalize(gCamera.getDirection() * glm::vec3(1, 0, 1));
  glm::vec3 cumulativeDirection = glm::vec3(0, 0, 0);
  if (state[SDL_SCANCODE_W]) {
    cumulativeDirection += forwardStepDirection;
  }
  if (state[SDL_SCANCODE_S]) {
    cumulativeDirection -= forwardStepDirection;
  }
  if (state[SDL_SCANCODE_A]) {
    cumulativeDirection += glm::vec3(forwardStepDirection.z, 0, -forwardStepDirection.x);
  }
  if (state[SDL_SCANCODE_D]) {
    cumulativeDirection += glm::vec3(-forwardStepDirection.z, 0, forwardStepDirection.x);
  }
  entityGod.getEntity("player").step(glm::normalize(cumulativeDirection) * acceleration);
  if (state[SDL_SCANCODE_SPACE]) {
    entityGod.getEntity("player").jump();
  }
}

/**
* Main Application Loop
* This is an infinite loop in our graphics application
*
* @return void
*/
void MainLoop(EntityGod &entityGod, Scene &scene){
    // Little trick to map mouse to center of screen always. Useful for handling 'mouselook'
    // This works because we effectively 're-center' our mouse at the start of every frame prior to detecting any mouse motion.
    SDL_WarpMouseInWindow(gGraphicsApplicationWindow,gScreenWidth/2,gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  int tick = 0;
	// While application is running
	while(!gQuit){
    tick += 1;
		// Handle Input
		Input(entityGod);
		// Setup anything (i.e. OpenGL State) that needs to take place before draw calls
    glm::vec3 pos = entityGod.getEntity("player").getPosition() * BLOCK_SCALE;
    gCamera.SetCameraEyePosition(pos.x, pos.y, pos.z);

    if (tick % 40 == 0) {
      entityGod.update();
    }
		// Draw Calls in OpenGL
		scene.draw();
		//Update screen of our specified window
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}

/**
* The last function called in the program
* This functions responsibility is to destroy any global
* objects in which we have create dmemory.
*
* @return void
*/
void CleanUp(){
	//Destroy our SDL2 Window
	SDL_DestroyWindow(gGraphicsApplicationWindow );
	gGraphicsApplicationWindow = nullptr;

	//Quit SDL subsystems
	SDL_Quit();
}

#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main(int argc, char* args[]) {
	// 1. Setup the graphics program
	InitializeProgram();

  std::vector<OBJModel> models;
  GravitySimulation simulation(0.0000001);
  Scene scene(gScreenWidth, gScreenHeight, gCamera);
  World world;
  RenderGod renderer(world, scene);
  TerrainGod generator(world);
  EntityGod entityManager(world);
  generator.generateSpawn();
  entityManager.createEntity(Player("player", {0, 0, 0}, 0, {0, 0, 0}));
  scene.createMesh("loaded", {});

	// 3. Create our graphics pipeline
	CreateGraphicsPipeline();

  renderer.setOrigin({0, 0, 0});
  renderer.setRadius(5);
  renderer.update();
	// 4. Call the main application loop
	MainLoop(entityManager, scene);

	// 5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}
