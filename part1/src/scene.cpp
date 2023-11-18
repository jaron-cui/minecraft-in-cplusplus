#include "scene.hpp"

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

Scene::Scene(GLuint* graphicsPipeline) {
  pipeline = graphicsPipeline;
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
GLint checkedUniformLocation(GLuint pipeline, std::string uniformName) {
  const GLchar* chars = uniformName.c_str();
  GLint u_Name = glGetUniformLocation(pipeline, chars);
  if (u_Name < 0){
    std::cout << "Could not find " << uniformName << ", maybe a mispelling?\n";
    exit(EXIT_FAILURE);
  }
  return u_Name;
}

// a convenient method for transferring lighting info to the shader
void setPointLightUniform(GLuint pipeline, PointLight light, int index) {
  std::string location = "u_pointLights[" + std::to_string(index) + "].";

  GLint u_lightColor = checkedUniformLocation(pipeline, location + "lightColor");
  glUniform3fv(u_lightColor, 1, &light.lightColor[0]);

  GLint u_lightPos = checkedUniformLocation(pipeline, location + "lightPos");
  glUniform3fv(u_lightPos, 1, &light.lightPos[0]);

  GLint u_ambientIntensity = checkedUniformLocation(pipeline, location + "ambientIntensity");
  glUniform1f(u_ambientIntensity, light.ambientIntensity);

  GLint u_specularStrength = checkedUniformLocation(pipeline, location + "specularStrength");
  glUniform1f(u_specularStrength, light.specularStrength);
}

void Scene::uploadUniforms() {
  int count = 0;
  for (auto light : lights) {
    setPointLightUniform(*pipeline, *light.second, count);
    count += 1;
  }
}

void predraw() {
  // position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (void*)0);
  // Normal information (nx,ny,nz)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (GLvoid*)(sizeof(GL_FLOAT)*6));
  // texture coordinate
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VBOVertex), (GLvoid*)(sizeof(GL_FLOAT)*9));
}

void postdraw() {
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void Scene::draw(){
  // Enable our attributes
	glBindVertexArray(vao);
  //Render data
  for (auto entry : meshes) {
    Mesh* mesh = entry.second;

    std::string diffusePath = mesh->getBaseModel().mtl.mapKD;
    if (textures.find(diffusePath) != textures.end()) {
      textures[diffusePath]->Bind(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->getVBO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getElementBuffer());
    predraw();
    glDrawElements(GL_TRIANGLES, mesh->getElementBufferSize(), GL_UNSIGNED_INT, (void*)(0 * sizeof(GLuint)));
    postdraw();
  }
	glBindVertexArray(0);

	// Stop using our current graphics pipeline
  glUseProgram(0);
}
