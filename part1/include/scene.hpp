#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include "obj.hpp"
#include "Texture.hpp"
#include "Camera.hpp"

// vvvvvvvvvvvvvvvvvvv Error Handling Routines vvvvvvvvvvvvvvv
static void GLClearAllErrors(){
  while (glGetError() != GL_NO_ERROR) {}
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
  if (GLenum error = glGetError()) {
    std::cout << "OpenGL Error:" << error << "\tLine: " << line << "\tfunction: " << function << std::endl;
    return true;
  }
  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);

/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string 
*/
std::string LoadShaderAsString(const std::string& filename);
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
GLuint CompileShader(GLuint type, const std::string& source);
/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);

/**
* Create the graphics pipeline
*
* @return void
*/
GLuint CreateGraphicsPipeline();

class Mesh {
  private:
  GLuint vbo = 0;
  size_t vboSize = 0;
  GLuint buffer = 0;
  size_t bufferSize = 0;
  OBJModel baseModel;
  float scale;
  glm::vec3 position;
  public:
  Mesh(GLuint vao, OBJModel model);
  void clearBuffers();

  void setVBOfromOBJ(OBJModel model);

  GLuint getVBO();

  size_t getVBOSize();

  GLuint getElementBuffer();

  size_t getElementBufferSize();

  glm::vec3 getPosition();

  void updateModel();

  void setScale(float factor);

  void setPosition(glm::vec3 offset);

  OBJModel& getBaseModel();
};

// represents a point light
struct PointLight{
    glm::vec3 lightColor;
    glm::vec3 lightPos;
    float ambientIntensity;

    float specularStrength;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class Scene {
  private:
    GLuint pipeline;
    int width, height;
    Camera &camera;
    float fov;
    GLuint vao;
    std::unordered_map<std::string, Mesh*> meshes;
    std::unordered_map<std::string, PointLight*> lights;
    std::unordered_map<std::string, Texture*> textures;
    void setupVertexArrayObject();
    void predraw();
    GLint checkedUniformLocation(std::string uniformName);
    void setPointLightUniform(PointLight light, int index);
  public:
    Scene(int width, int height, Camera &camera);
    ~Scene();
    bool createMesh(std::string name, OBJModel obj);
    Mesh* getMesh(std::string name);
    void deleteMesh(std::string name);
    bool createLight(std::string name, PointLight light);
    PointLight* getLight(std::string name);
    void deleteLight(std::string name);
    void uploadUniforms();
    void draw();
    void setFOV(float newFOV);
};

// get the uniform location and run generic checks
GLint checkedUniformLocation(GLuint pipeline, std::string uniformName);

// a structure representing a vertex entry in a VBO
struct VBOVertex {
  float x, y, z;
  float nx, ny, nz;
  float tx, ty;
  VBOVertex(glm::vec3 position, glm::vec3 normal, glm::vec2 tc) {
    x = position.x;
    y = position.y;
    z = position.z;
    nx = normal.x;
    ny = normal.y;
    nz = normal.z;
    tx = tc.x;
    ty = tc.y;
  }

  inline bool operator==(const VBOVertex &other) const {
      // bool comparison = result of comparing 'this' to 'other'
      return x == other.x && y == other.y && z == other.z
          && nx == other.nx && ny == other.ny && nz == other.nz
          && tx == other.tx && ty == other.ty;
  }
};

namespace std {
  template<>
  struct hash<VBOVertex> {
    inline size_t operator()(const VBOVertex& x) const {
      return x.x + x.y * 3 + x.z * 5
          + x.nx * 17 + x.ny * 19 + x.nz * 23
          + x.tx * 29 + x.ty * 31;
    }
  };
}