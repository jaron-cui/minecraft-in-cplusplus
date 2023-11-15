#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Triangle{
    glm::vec3 vertices[3]; // 3 vertices per triangle
};

struct VertexDescriptor {
    int vertex;
    int vertexNormal;
    int textureCoordinate;
};

struct Face {
  std::vector<VertexDescriptor> vertexDescriptors;
};

struct TextureCoordinate {
  float x, y;
};

struct MTL {
  std::string mapKD;
};

struct OBJModel {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> vertexNormals;
    std::vector<Face> faces;
    std::vector<TextureCoordinate> textureCoordinates;
    MTL mtl;
};

// scale an OBJ from the origin by a certain factor
OBJModel scaleOBJ(OBJModel model, float factor);

// translate all vertices of an OBJ by a constant amount
OBJModel offsetOBJ(OBJModel model, glm::vec3 offset);

// load a .MTL file
bool loadMTL(MTL &mtl, std::string mtlFile);

// load an OBJ file
OBJModel loadOBJ(std::string path);

OBJModel UNIT_CUBE();