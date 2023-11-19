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
#include <chrono>

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

const int FRAMERATE = 120;
const int FRAMETIME_MS = std::floor(1000.0 / FRAMERATE);

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

struct Game {
  World &world;
  Scene &scene;
  TerrainGod &terrainGod;
  EntityGod &entityGod;
  RenderGod &renderGod;
  int timeSprinting = 0;
};

/**
* Function called in the Main application loop to handle user input
*
* @return void
*/
void Input(Game &game) {
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
  int sprintTransitionTime = float(FRAMERATE) / 12;

  float acceleration;
  float maxSpeed;
  float fov = glm::smoothstep(0.0f, float(sprintTransitionTime), float(game.timeSprinting)) * 10.0f + 45.0f;
  if (state[SDL_SCANCODE_LCTRL]) {
    acceleration = 0.011;
    maxSpeed = 0.078;
    game.timeSprinting += game.timeSprinting < sprintTransitionTime ? 1 : 0;
  } else {
    acceleration = 0.008;
    maxSpeed = 0.06;
    game.timeSprinting -= game.timeSprinting > 0 ? 1 : 0;
  }
  game.scene.setFOV(glm::radians(fov));

  // Camera
  // Update our position of the camera

  glm::vec3 forwardStepDirection = glm::normalize(gCamera.getDirection() * glm::vec3(1, 0, 1));
  glm::vec3 cumulativeDirection = glm::vec3(0, 0, 0);
  struct WalkKeyBind { SDL_Scancode key; glm::vec3 step; };
  WalkKeyBind walkBinds[4] = {
    {SDL_SCANCODE_W, forwardStepDirection}, {SDL_SCANCODE_S, -forwardStepDirection},
    {SDL_SCANCODE_A, glm::vec3(forwardStepDirection.z, 0, -forwardStepDirection.x)},
    {SDL_SCANCODE_D, glm::vec3(-forwardStepDirection.z, 0, forwardStepDirection.x)}
    };
  for (WalkKeyBind walkBind : walkBinds) {
    if (state[walkBind.key]) {
      cumulativeDirection += walkBind.step;
    }
  }
  Entity &player = game.entityGod.getEntity("player");
  player.setMaxMovementSpeed(maxSpeed);
  player.step(glm::normalize(cumulativeDirection) * acceleration);
  if (state[SDL_SCANCODE_SPACE]) {
    player.jump();
  }
}

/**
* Main Application Loop
* This is an infinite loop in our graphics application
*
* @return void
*/
void MainLoop(Game &game){
    // Little trick to map mouse to center of screen always. Useful for handling 'mouselook'
    // This works because we effectively 're-center' our mouse at the start of every frame prior to detecting any mouse motion.
    SDL_WarpMouseInWindow(gGraphicsApplicationWindow,gScreenWidth/2,gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  int tick = 0;
  // process physics once per tick
  int physicsTick = 1;
  // update rendering once every 2 seconds
  int renderTick = FRAMERATE * 2;
  // update generation once every 5 seconds
  int generationTick = FRAMERATE * 5;
	// While application is running
  std::thread renderThread;
  std::thread terrainGenerationThread;
  // thread geenrationthread;
	while(!gQuit){
    tick += 1;
    const auto frameEnd = std::chrono::steady_clock::now() + std::chrono::milliseconds(FRAMETIME_MS);
		// Handle Input
		Input(game);
		// Setup anything (i.e. OpenGL State) that needs to take place before draw calls
    Entity &player = game.entityGod.getEntity("player");
    glm::vec3 cameraOffset = glm::vec3(POSY) * player.getHitbox().dimensions.y * 0.35333f;
    glm::vec3 pos = (player.getPosition() + cameraOffset) * BLOCK_SCALE;
    gCamera.SetCameraEyePosition(pos.x, pos.y, pos.z);

    if (tick % physicsTick == 0) {
      game.entityGod.update();
    }
    if (tick % renderTick == 0) {
      // wrap up rendering stuff
      if (renderThread.joinable()) {
        renderThread.join();
      }
      game.renderGod.setOrigin(player.getPosition());
      game.renderGod.cullFarChunks(2);
      game.renderGod.uploadCache();
      // transfer rendered chunks to vbo
      renderThread = std::thread(&RenderGod::update, &game.renderGod);
    }
    if (tick % generationTick == 0) {
      if (terrainGenerationThread.joinable()) {
        terrainGenerationThread.join();
      }
      game.terrainGod.setOrigin(player.getPosition());
      // geenrationthread.startandinthnewthings()
      // game.terrainGod.update();
      terrainGenerationThread = std::thread(&TerrainGod::update, &game.terrainGod);
    }
		// Draw Calls in OpenGL
		game.scene.draw();
		//Update screen of our specified window
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    std::this_thread::sleep_until(frameEnd);
	}
  if (renderThread.joinable()) {
    renderThread.join();
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


/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main(int argc, char* args[]) {
	// 1. Setup the graphics program
	InitializeProgram();

  Scene scene(gScreenWidth, gScreenHeight, gCamera);
  World world(time(NULL));
  RenderGod renderer(world, scene);
  TerrainGod generator(world);
  EntityGod entityManager(world);
  Game game = {world, scene, generator, entityManager, renderer};
  //generator.generateSpawn();
  entityManager.createEntity(Player("player", {0, 0, 0}, 0, {0, 0, 0}));

	// 3. Create our graphics pipeline
	CreateGraphicsPipeline();

  generator.generateSpawn();
  generator.setOrigin({0, 0, 0});
  generator.setRadius(4);
  generator.update();

  renderer.setOrigin({0, 0, 0});
  renderer.setRadius(2);
  renderer.update();
	// 4. Call the main application loop
	MainLoop(game);

	// 5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}
