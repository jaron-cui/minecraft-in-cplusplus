#include "scene.hpp"

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

GLuint CreateGraphicsPipeline() {
  std::string vertexShaderSource = LoadShaderAsString("./shaders/vert.glsl");
  std::string fragmentShaderSource = LoadShaderAsString("./shaders/frag.glsl");

	return CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}

// encode an OBJ into VBO data
bool encodeOBJ(OBJModel model, std::vector<VBOVertex> &data, std::vector<GLuint> &indices) {
  std::unordered_map<VBOVertex, int> processedVertices;

  for (Face face : model.faces) {
    for (VertexDescriptor vd : face.vertexDescriptors) {
      if (vd.vertex < 0 || vd.vertex >= model.vertices.size()) {
        return false;
      }
      if (model.mtl.mapKD.length() > 0 && (vd.textureCoordinate < 0 || vd.textureCoordinate >= model.textureCoordinates.size())) {
        return false;
      }
      if (vd.vertexNormal < 0 || vd.vertexNormal >= model.vertexNormals.size()) {
        return false;
      }
      glm::vec2 b = {0, 0};
      glm::vec2 tc = model.mtl.mapKD.length() == 0 ? b : model.textureCoordinates.at(vd.textureCoordinate);
      VBOVertex v = VBOVertex(
        model.vertices.at(vd.vertex),
        model.vertexNormals.at(vd.vertexNormal),
        tc
        );
      int index;
      if (processedVertices.find(v) == processedVertices.end()) {
        // if not pre-existing
        index = data.size();
        data.push_back(v);
        processedVertices[v] = index;
      } else {
        // if already exists
        index = processedVertices.at(v);
      }
      indices.push_back(index);
    }
  }
  return true;
}

void Mesh::setVBOfromOBJ(OBJModel model) {
  std::vector<VBOVertex> data;
  std::vector<GLuint> indices;
  if (!encodeOBJ(model, data, indices)) {
    throw std::invalid_argument("Invalid OBJ cannot be loaded into VBO.");
  }
  vboSize = data.size();
  bufferSize = indices.size();
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 						// Kind of buffer we are working with 
                                            // (e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
              data.size() * sizeof(VBOVertex), 	// Size of data in bytes
              data.data(), 											// Raw array of data
              GL_STATIC_DRAW);
  // set up indexing
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
}

GLuint Mesh::getVBO() {
  return vbo;
}

size_t Mesh::getVBOSize() {
  return vboSize;
}

GLuint Mesh::getElementBuffer() {
  return buffer;
}

size_t Mesh::getElementBufferSize() {
  return bufferSize;
}

glm::vec3 Mesh::getPosition() {
  return position;
}

OBJModel& Mesh::getBaseModel() {
  return baseModel;
}

void Mesh::updateModel() {
  setVBOfromOBJ(offsetOBJ(scaleOBJ(baseModel, scale), position));
}

void Mesh::setScale(float factor) {
  scale = factor;
  updateModel();
}

void Mesh::setPosition(glm::vec3 offset) {
  position = offset;
  updateModel();
}

Mesh::Mesh(GLuint vao, OBJModel model) {
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &buffer);

  glBindVertexArray(0);

  baseModel = model;
  setVBOfromOBJ(model);
}

void Mesh::clearBuffers() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &buffer);
}

Scene::Scene(int w, int h, Camera &camera): camera(camera) {
  pipeline = CreateGraphicsPipeline();
  width = w;
  height = h;
  setupVertexArrayObject();
}

Scene::~Scene() {
  for (auto mesh : meshes) {
    deleteMesh(mesh.first);
  }
  for (auto light : lights) {
    deleteLight(light.first);
  }
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(pipeline);
}

void Scene::setupVertexArrayObject() {
  // Vertex Arrays Object (VAO) Setup
	glGenVertexArrays(1, &vao);
	// We bind (i.e. select) to the Vertex Array Object (VAO) that we want to work withn.
	glBindVertexArray(vao);
	// Unbind our currently bound Vertex Array Object
	glBindVertexArray(0);
}

bool tryLoadingTexture(std::string path, std::unordered_map<std::string, Texture*> &map) {
  if (path.length() == 0 || map.find(path) != map.end()) {
    return false;
  }

  Texture *t = new Texture();
  t->LoadTexture(path);
  std::cout << "binding " << path << " to slot " << map.size() << std::endl;
  t->Bind(map.size());
  map[path] = t;
  return true;
}

bool Scene::createMesh(std::string name, OBJModel obj) {
  if (meshes.find(name) != meshes.end()) {
    return false;
  }
  // std::cout << "creating mesh" << std::endl;
  Mesh* mesh = new Mesh(vao, obj);
  // std::cout << "storing mesh" << std::endl;
  meshes[name] = mesh;
  tryLoadingTexture(obj.mtl.mapKD, textures);
  
  // std::cout << "done storing mesh" << std::endl;
  return true;
}

Mesh* Scene::getMesh(std::string name) {
  return meshes[name];
}

void Scene::deleteMesh(std::string name) {
  if (meshes.find(name) == meshes.end()) {
    return;
  }
  meshes[name]->clearBuffers();
  delete &meshes[name];
  meshes.erase(name);
}

bool Scene::createLight(std::string name, PointLight data) {
  if (lights.find(name) != lights.end()) {
    return false;
  }
  PointLight* light = (PointLight*) malloc(sizeof(PointLight));
  memcpy(light, &data, sizeof(PointLight));
  lights[name] = light;
  return true;
}

PointLight* Scene::getLight(std::string name) {
  return lights[name];
}

void Scene::deleteLight(std::string name) {
  if (lights.find(name) == lights.end()) {
    return;
  }
  delete &lights[name];
  lights.erase(name);
}

// get the uniform location and run generic checks
GLint Scene::checkedUniformLocation(std::string uniformName) {
  const GLchar* chars = uniformName.c_str();
  GLint u_Name = glGetUniformLocation(pipeline, chars);
  if (u_Name < 0){
    std::cout << "Could not find " << uniformName << ", maybe a mispelling?\n";
    exit(EXIT_FAILURE);
  }
  return u_Name;
}

// a convenient method for transferring lighting info to the shader
void Scene::setPointLightUniform(PointLight light, int index) {
  std::string location = "u_pointLights[" + std::to_string(index) + "].";

  GLint u_lightColor = checkedUniformLocation(location + "lightColor");
  glUniform3fv(u_lightColor, 1, &light.lightColor[0]);

  GLint u_lightPos = checkedUniformLocation(location + "lightPos");
  glUniform3fv(u_lightPos, 1, &light.lightPos[0]);

  GLint u_ambientIntensity = checkedUniformLocation(location + "ambientIntensity");
  glUniform1f(u_ambientIntensity, light.ambientIntensity);

  GLint u_specularStrength = checkedUniformLocation(location + "specularStrength");
  glUniform1f(u_specularStrength, light.specularStrength);
}

void Scene::uploadUniforms() {
  int count = 0;
  for (auto light : lights) {
    setPointLightUniform(*light.second, count);
    count += 1;
  }
}

void Scene::predraw() {
  glEnable(GL_DEPTH_TEST);                    // NOTE: Need to enable DEPTH Test
  glEnable(GL_CULL_FACE);

  // Set the polygon fill mode
  glPolygonMode(GL_FRONT, GL_FILL);

  // Initialize clear color
  // This is the background of the screen.
  glViewport(0, 0, width, height);
  glClearColor( 0.1f, 4.f, 7.f, 1.f );

  //Clear color buffer and Depth Buffer
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // Use our shader
	glUseProgram(pipeline);

  // Model transformation by translating our object into world space
  glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,0.0f)); 

  // Retrieve our location of our Model Matrix
  GLint u_ModelMatrixLocation = checkedUniformLocation("u_ModelMatrix");
  glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);

  // Update the View Matrix
  GLint u_ViewMatrixLocation = checkedUniformLocation("u_ViewMatrix");
  glm::mat4 viewMatrix = camera.GetViewMatrix();
  glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);

  // Projection matrix (in perspective) 
  glm::mat4 perspective = glm::perspective(
    glm::radians(45.0f), (float) width / height, 0.1f, 20.0f);

  // Retrieve our location of our perspective matrix uniform 
  GLint u_ProjectionLocation = checkedUniformLocation("u_Projection");
  glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);

  GLint u_viewPosition = checkedUniformLocation("u_viewPosition");
  glm::vec3 cameraPosition = glm::vec3(camera.getPosition());
  glUniform3fv(u_viewPosition, 1, &cameraPosition[0]);

  GLint u_DiffuseTexture = checkedUniformLocation("u_DiffuseTexture");
  glUniform1i(u_DiffuseTexture, 0);
}

void setupVAO() {
  // position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (void*)0);
  // Normal information (nx,ny,nz)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (GLvoid*)(sizeof(GL_FLOAT)*3));
  // texture coordinate
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (GLvoid*)(sizeof(GL_FLOAT)*6));
}

void closeVAO() {
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void Scene::draw(){
  predraw();
  // Enable our attributes
	glBindVertexArray(vao);
  uploadUniforms();
  //Render data
  for (auto entry : meshes) {
    Mesh* mesh = entry.second;

    std::string diffusePath = mesh->getBaseModel().mtl.mapKD;
    if (textures.find(diffusePath) != textures.end()) {
      textures[diffusePath]->Bind(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->getVBO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getElementBuffer());
    setupVAO();
    glDrawElements(GL_TRIANGLES, mesh->getElementBufferSize(), GL_UNSIGNED_INT, (void*)(0 * sizeof(GLuint)));
    closeVAO();
  }
	glBindVertexArray(0);

	// Stop using our current graphics pipeline
  glUseProgram(0);
}
