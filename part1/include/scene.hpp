#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "obj.hpp"
#include "Texture.hpp"

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
    GLuint* pipeline;
    GLuint* vao;
    std::unordered_map<std::string, Mesh*> meshes;
    std::unordered_map<std::string, PointLight*> lights;
    std::unordered_map<std::string, Texture*> textures;
  public:
    Scene(GLuint* graphicsPipeline, GLuint* vertexArrayObject);
    ~Scene();
    bool createMesh(std::string name, OBJModel obj);
    Mesh* getMesh(std::string name);
    void deleteMesh(std::string name);
    bool createLight(std::string name, PointLight light);
    PointLight* getLight(std::string name);
    void deleteLight(std::string name);
    void uploadUniforms();
    void draw();
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

// struct VBOVertex {
//   float x, y, z;
//   float nx, ny, nz;
//   uint8_t tile;
//   VBOVertex(glm::vec3 position, glm::vec3 normal, uint8_t blockType) {
//     x = position.x;
//     y = position.y;
//     z = position.z;
//     nx = normal.x;
//     ny = normal.y;
//     nz = normal.z;
//     tile = blockType;
//   }

//   inline bool operator==(const VBOVertex &other) const {
//       // bool comparison = result of comparing 'this' to 'other'
//       // return x == other.x && y == other.y && z == other.z
//       //     && r == other.r && g == other.g && b == other.b
//       //     && nx == other.nx && ny == other.ny && nz == other.nz
//       //     && tx == other.tx && ty == other.ty;
//       return x == other.x && y == other.y && z == other.z
//           && nx == other.nx && ny == other.ny && nz == other.nz
//           && tile == other.tile;
//   }
// };

// namespace std {
//   template<>
//   struct hash<VBOVertex> {
//     inline size_t operator()(const VBOVertex& x) const {
//       return x.x + x.y * 3 + x.z * 5
//           // + x.r * 7 + x.g * 11 + x.b * 13
//           + x.nx * 17 + x.ny * 19 + x.nz * 23
//           // + x.tx * 29 + x.ty * 31;
//           + x.tile * 43;
//     }
//   };
// }