#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <bits/stdc++.h>

namespace std {
  template<>
  struct hash<glm::ivec3> {
    inline size_t operator()(const glm::ivec3& x) const {
      return x.x * 5 + x.y * 17 + x.z * 37;
    }
  };

  template<>
  struct hash<glm::vec3> {
    inline size_t operator()(const glm::vec3& x) const {
      return x.x * 5 + x.y * 17 + x.z * 37;
    }
  };
}

struct VertexDescriptor {
    int vertex;
    int vertexNormal;
    int textureCoordinate;
};

struct Face {
  std::vector<VertexDescriptor> vertexDescriptors;
};

struct MTL {
  std::string mapKD;
};

struct OBJModel {
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> vertexNormals;
  std::vector<Face> faces;
  std::vector<glm::vec2> textureCoordinates;
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

struct OBJBuilder {
  OBJModel model;
  // add a vertex to the OBJModel, if it isn't already present
  // return the index of the vertex
  int addVertex(glm::vec3 vertex) {
    std::vector<glm::vec3> &v = model.vertices;
    auto it = std::find(v.begin(), v.end(), vertex);
    // if vertex does not already exit, insert it
    if (it == v.end()) {
      v.push_back(vertex);
      return v.size() - 1;
    }
    // if vertex does exist, return existing index
    return it - v.begin();
  }

  int addNormal(glm::vec3 normal) {
    std::vector<glm::vec3> &v = model.vertexNormals;
    auto it = std::find(v.begin(), v.end(), normal);
    // if vertex does not already exit, insert it
    if (it == v.end()) {
      v.push_back(normal);
      return v.size() - 1;
    }
    // if vertex does exist, return existing index
    return it - v.begin();
  }

  int addTextureCoordinate(glm::vec2 textureCoordinate) {
    std::vector<glm::vec2> &v = model.textureCoordinates;
    auto it = std::find(v.begin(), v.end(), textureCoordinate);
    // if vertex does not already exit, insert it
    if (it == v.end()) {
      v.push_back(textureCoordinate);
      return v.size() - 1;
    }
    // if vertex does exist, return existing index
    return it - v.begin();
  }

  void addSimpleFace(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> textureCoordinates) {
    std::vector<VertexDescriptor> vertexDescriptors;
    for (int i = 0; i < vertices.size(); i += 1) {
      vertexDescriptors.push_back({addVertex(vertices[i]), addNormal(normals[i]), addTextureCoordinate(textureCoordinates[i])});
    }
    model.faces.push_back({vertexDescriptors});
  }
};