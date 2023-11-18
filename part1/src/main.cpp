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
int gScreenWidth 						= 640;
int gScreenHeight 						= 480;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext			= nullptr;

// Main loop flag
bool gQuit = false; // If this is quit = 'true' then the program terminates.

// shader
// The following stores the a unique id for the graphics pipeline
// program object that will be used for our OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram	= 0;

// OpenGL Objects
// Vertex Array Object (VAO)
GLuint vertexArrayObject = 0;
// Vertex Buffer Object (VBO)
GLuint  gVertexBufferObjectFloor            = 0;

const int vertexDataSize = 11;

// Camera
Camera gCamera;

// Draw wireframe mode
GLenum gPolygonMode = GL_FILL;

// Floor resolution
size_t gFloorResolution = 10;
size_t gFloorTriangles  = 0;

std::unordered_map<std::string, Texture*> gTextures = {};

// ^^^^^^^^^^^^^^^^^^^^^^^^ Globals ^^^^^^^^^^^^^^^^^^^^^^^^^^^

// vvvvvvvvvvvvvvvvvvv Error Handling Routines vvvvvvvvvvvvvvv
static void GLClearAllErrors(){
  while(glGetError() != GL_NO_ERROR){
  }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
  while (GLenum error = glGetError()) {
    std::cout << "OpenGL Error:" << error << "\tLine: " << line << "\tfunction: " << function << std::endl;
    return true;
  }
  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);
// ^^^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^

/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string 
*/
std::string LoadShaderAsString(const std::string& filename){
  // Resulting shader program loaded as a single string
  std::string result = "";

  std::string line = "";
  std::ifstream myFile(filename.c_str());

  if (myFile.is_open()) {
    while (std::getline(myFile, line)) {
      result += line + '\n';
    }
    myFile.close();
  }

  return result;
}

/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*	    Compile a vertex shader: 	CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader: 	CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
	GLuint shaderObject;

	// Based on the type passed in, we create a shader object specifically for that
	// type.
	if (type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	} else if (type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of our shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile our shader
	glCompileShader(shaderObject);

	// Retrieve the result of our compilation
	int result;
	// Our goal with glGetShaderiv is to retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE){
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length]; // Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if (type == GL_VERTEX_SHADER){
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
		} else if (type == GL_FRAGMENT_SHADER){
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
		}
		// Reclaim our memory
		delete[] errorMessages;

		// Delete our broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

  return shaderObject;
}

/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){
  // Create a new program object
  GLuint programObject = glCreateProgram();

  // Compile our shaders
  GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  // Link our two shader programs together.
	// Consider this the equivalent of taking two .cpp files, and linking them into
	// one executable file.
  glAttachShader(programObject,myVertexShader);
  glAttachShader(programObject,myFragmentShader);
  glLinkProgram(programObject);

  // Validate our program
  glValidateProgram(programObject);

  // Once our final program Object has been created, we can
	// detach and then delete our individual shaders.
  glDetachShader(programObject,myVertexShader);
  glDetachShader(programObject,myFragmentShader);
	// Delete the individual shaders once we are done
  glDeleteShader(myVertexShader);
  glDeleteShader(myFragmentShader);

  return programObject;
}

/**
* Create the graphics pipeline
*
* @return void
*/
void CreateGraphicsPipeline(){
  std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
  std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");

	gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}


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
* Setup your geometry during the vertex specification step
*
* @return void
*/
/**
* Setup your geometry during the vertex specification step
*
* @return void
*/
void VertexSpecification(){
	// Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &vertexArrayObject);
	// We bind (i.e. select) to the Vertex Array Object (VAO) that we want to work withn.
	glBindVertexArray(vertexArrayObject);
	// Vertex Buffer Object (VBO) creation
	glGenBuffers(1, &gVertexBufferObjectFloor);
	// Unbind our currently bound Vertex Array Object
	glBindVertexArray(0);
}

// get the uniform location and run generic checks
GLint checkedUniformLocation(std::string uniformName) {
  const GLchar* chars = uniformName.c_str();
  GLint u_Name = glGetUniformLocation(gGraphicsPipelineShaderProgram, chars);
  if (u_Name < 0){
    std::cout << "Could not find " << uniformName << ", maybe a mispelling?\n";
    exit(EXIT_FAILURE);
  }
  return u_Name;
}

/**
* PreDraw
* Typically we will use this for setting some sort of 'state'
* Note: some of the calls may take place at different stages (post-processing) of the
* 		 pipeline.
* @return void
*/
void PreDraw(Scene* scene){
	// Disable depth test and face culling.
  glEnable(GL_DEPTH_TEST);                    // NOTE: Need to enable DEPTH Test
  glEnable(GL_CULL_FACE);

  // Set the polygon fill mode
  glPolygonMode(GL_FRONT_AND_BACK,gPolygonMode);

  // Initialize clear color
  // This is the background of the screen.
  glViewport(0, 0, gScreenWidth, gScreenHeight);
  glClearColor( 0.1f, 4.f, 7.f, 1.f );

  //Clear color buffer and Depth Buffer
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // Use our shader
	glUseProgram(gGraphicsPipelineShaderProgram);

  // Model transformation by translating our object into world space
  glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,0.0f)); 

  // Retrieve our location of our Model Matrix
  GLint u_ModelMatrixLocation = checkedUniformLocation("u_ModelMatrix");
  glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);

  // Update the View Matrix
  GLint u_ViewMatrixLocation = checkedUniformLocation("u_ViewMatrix");
  glm::mat4 viewMatrix = gCamera.GetViewMatrix();
  glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);

  // Projection matrix (in perspective) 
  glm::mat4 perspective = glm::perspective(
    glm::radians(45.0f), (float) gScreenWidth / gScreenHeight, 0.1f, 20.0f);

  // Retrieve our location of our perspective matrix uniform 
  GLint u_ProjectionLocation = checkedUniformLocation("u_Projection");
  glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);

  GLint u_viewPosition = checkedUniformLocation("u_viewPosition");
  glm::vec3 cameraPosition = glm::vec3(gCamera.getPosition());
  glUniform3fv(u_viewPosition, 1, &cameraPosition[0]);

  scene->uploadUniforms();

  GLint u_DiffuseTexture = checkedUniformLocation("u_DiffuseTexture");
  glUniform1i(u_DiffuseTexture, 0);
}

/**
* Draw
* The render function gets called once per loop.
* Typically this includes 'glDraw' related calls, and the relevant setup of buffers
* for those calls.
*
* @return void
*/
void Draw(Scene* scene){
  scene->draw();
}

/**
* Function called in the Main application loop to handle user input
*
* @return void
*/
void Input(Scene* scene, std::vector<OBJModel> models, EntityGod &entityGod){
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

class ProgramSession {
  public:
  std::vector<OBJModel> models;
  unsigned int tickCount = 0;
  GravitySimulation *gravitySimulation;
  Scene *scene;
  ProgramSession(std::vector<OBJModel> &objModels, GravitySimulation* simulation, Scene* s) {
    models = objModels;
    gravitySimulation = simulation;
    scene = s;
  }

  void tick() {
    tickCount += 1;
    if (tickCount < 1000 || tickCount % 5 != 0) {
      return;
    }
  }
};

/**
* Main Application Loop
* This is an infinite loop in our graphics application
*
* @return void
*/
void MainLoop(ProgramSession* state, EntityGod &entityGod){
    // Little trick to map mouse to center of screen always. Useful for handling 'mouselook'
    // This works because we effectively 're-center' our mouse at the start of every frame prior to detecting any mouse motion.
    SDL_WarpMouseInWindow(gGraphicsApplicationWindow,gScreenWidth/2,gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  int tick = 0;
	// While application is running
	while(!gQuit){
    tick += 1;
		// Handle Input
		Input(state->scene, state->models, entityGod);
		// Setup anything (i.e. OpenGL State) that needs to take place before draw calls
    state->tick();
    glm::vec3 pos = entityGod.getEntity("player").getPosition() * BLOCK_SCALE;
    gCamera.SetCameraEyePosition(pos.x, pos.y, pos.z);

    if (tick % 40 == 0) {
      entityGod.update();
    }
		PreDraw(state->scene);
		// Draw Calls in OpenGL
		Draw(state->scene);
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

    // Delete our OpenGL Objects
  glDeleteBuffers(1, &gVertexBufferObjectFloor);
  glDeleteVertexArrays(1, &vertexArrayObject);

	// Delete our Graphics pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);

	//Quit SDL subsystems
	SDL_Quit();
}


bool tryLoadingTexture(std::string path) {
  std::cout << "Loading texture at " << path << std::endl;
  if (gTextures.find(path) != gTextures.end()) {
    return false;
  }

  Texture *t = new Texture();
  t->LoadTexture(path);
  t->Bind(gTextures.size());
  gTextures[path] = t;
  return true;
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
	
	// 2. Setup our geometry
	VertexSpecification();

  std::vector<OBJModel> models;
  GravitySimulation simulation(0.0000001);
  Scene scene(&gGraphicsPipelineShaderProgram, &vertexArrayObject);
  ProgramSession state(models, &simulation, &scene);
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
	MainLoop(&state, entityManager);

	// 5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}
