#include "obj.hpp"

#include <iostream>
#include <fstream>

OBJModel scaleOBJ(OBJModel model, float factor) {
  std::vector<glm::vec3> scaled;
  for (glm::vec3 v : model.vertices) {
    scaled.push_back(v * factor);
  }
  return {scaled, model.vertexNormals, model.faces};
}

OBJModel offsetOBJ(OBJModel model, glm::vec3 offset) {
  std::vector<glm::vec3> translated;
  for (glm::vec3 v : model.vertices) {
    translated.push_back(v + offset);
  }
  return {translated, model.vertexNormals, model.faces};
}

void split(const std::string& s, std::vector<std::string>& results, const std::string delimiter) {
  // represents the next initial free character
  int lastDelimiter = 0;
  while (true) {
    int nextDelimiter = s.find(delimiter, lastDelimiter);
    // std::cout << "delim " << lastDelimiter << " " << nextDelimiter << std::endl;
    // if there is no more delimiter left
    if (nextDelimiter == -1) {
      // parse the last token if there is one before exiting
      if (s.length() - lastDelimiter > 0) {
        // std::cout << "last" << s.substr(lastDelimiter) << std::endl;
        results.push_back(s.substr(lastDelimiter));
      }
      break;
    }
    // if there is a non-whitespace substring, add it to the vector
    if (nextDelimiter >= lastDelimiter) {
      // std::cout << "adding " << s.substr(lastDelimiter, nextDelimiter - lastDelimiter) << std::endl;
      results.push_back(s.substr(lastDelimiter, nextDelimiter - lastDelimiter));
    }
    lastDelimiter = nextDelimiter + delimiter.length();
  }
}

glm::vec3 extractVertex(const std::string line) {
  std::vector<std::string> tokens;
  if (line.length() < 3) {
    throw std::invalid_argument("Invalid line in OBJ - no values.");
  }
  split(line.substr(line.find(' ') + 1), tokens, " ");
  if (tokens.size() != 3) {
    throw std::invalid_argument("Invalid line in OBJ - incorrect number of values.");
  }
  return {std::stof(tokens.at(0)), std::stof(tokens.at(1)), std::stof(tokens.at(2))};
}

TextureCoordinate extractTextureCoordinate(const std::string line) {
  std::vector<std::string> tokens;
  if (line.length() < 3) {
    throw std::invalid_argument("Invalid line in OBJ - no values.");
  }
  split(line.substr(line.find(' ') + 1), tokens, " ");
  if (tokens.size() != 2) {
    throw std::invalid_argument("Invalid line in OBJ - incorrect number of values.");
  }
  return {std::stof(tokens.at(0)), std::stof(tokens.at(1))};
}

std::string extractMTLLIB(const std::string line) {
  std::vector<std::string> tokens;
  if (line.length() < 8) {
    throw std::invalid_argument("Invalid line in OBJ - no values.");
  }
  split(line.substr(line.find(' ') + 1), tokens, " ");
  if (tokens.size() != 1) {
    throw std::invalid_argument("Invalid line in OBJ - incorrect number of values.");
  }
  return tokens.at(0);
}

Face extractFace(const std::string line) {
  // validate face representation
  std::vector<std::string> tokens;
  if (line.length() < 3) {
    throw std::invalid_argument("Invalid line in OBJ - no values.");
  }
  split(line.substr(2), tokens, " ");
  if (tokens.size() < 3) {
    throw std::invalid_argument("Invalid line in OBJ - incorrect number of values.");
  }
  // gather the specified vertex/vertex normal pairs into a face struct
  std::vector<VertexDescriptor> descriptors;
  for (std::string token : tokens) {
    std::vector<std::string> indices;
    split(token, indices, "/");
    if (indices.size() != 3) {
      throw std::invalid_argument("Invalid face in OBJ - there should be a vertex and normal index.");
    }
    descriptors.push_back(VertexDescriptor{stoi(indices.at(0)) - 1, stoi(indices.at(2)) - 1, indices.at(1) == "" ? -1 : stoi(indices.at(1)) - 1});
  }
  return Face{descriptors};
}

void updateMTL(MTL &mtl, const std::string line, const std::string path) {
  if (line.length() < 8) {
    return;
  }
  std::vector<std::string> tokens;
  split(line, tokens, " ");
  if (line.substr(0, 7) == "map_Kd " && tokens.size() == 2) {
    int folder = path.find_last_of("/\\");
    mtl.mapKD = (folder == -1 ? "" : path.substr(0, folder + 1)) + tokens.at(1);
  }
}

bool loadMTL(MTL &mtl, std::string mtlFile) {
  std::ifstream in;
  std::string line;
  in.open(mtlFile);
  if (!in.is_open()) {
    return false;
  }
  while (getline(in, line)) {
    updateMTL(mtl, line, mtlFile);
  }
  in.close();
  return true;
}

void updateOBJModel(OBJModel &model, const std::string line, const std::string path) {
  if (line.length() < 2) {
    return;
  }
  if (line.substr(0, 2) == "v ") {
    model.vertices.push_back(extractVertex(line));
  } else if (line.substr(0, 2) == "f ") {
    model.faces.push_back(extractFace(line));
  } else if (line.length() >= 3) {
    if (line.substr(0, 3) == "vn ") {
      model.vertexNormals.push_back(extractVertex(line));
    } else if (line.substr(0, 3) == "vt ") {
      model.textureCoordinates.push_back(extractTextureCoordinate(line));
    } else if (line.length() >= 7 && line.substr(0, 7) == "mtllib ") {
      int folder = path.find_last_of("/\\");
      std::string mtl = (folder == -1 ? "" : path.substr(0, folder + 1)) +  extractMTLLIB(line);
      loadMTL(model.mtl, mtl);
    }
  }
}

OBJModel loadOBJ(std::string path) {
  OBJModel model;
  std::ifstream in;
  std::string line;
  in.open(path);
  if (!in.is_open()) {
    throw std::invalid_argument("File not found.");
  }
  while (getline(in, line)) {
    updateOBJModel(model, line, path);
  }
  in.close();
  return model;
}

OBJModel UNIT_CUBE() {
  std::vector<glm::vec3> vertices{{1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1}};
  std::vector<glm::vec3> vertexNormals{{1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1}};
  
  std::vector<Face> faces{
      {{{0, 0}, {1, 1}, {3, 3}}}, {{{0, 0}, {3, 3}, {2, 2}}},
      {{{0, 0}, {4, 4}, {1, 1}}}, {{{4, 4}, {5, 5}, {1, 1}}},
      {{{2, 2}, {3, 3}, {6, 6}}}, {{{6, 6}, {3, 3}, {7, 7}}},
      {{{6, 6}, {5, 5}, {4, 4}}}, {{{6, 6}, {7, 7}, {5, 5}}},
      {{{6, 6}, {0, 0}, {2, 2}}}, {{{6, 6}, {4, 4}, {0, 0}}},
      {{{7, 7}, {3, 3}, {1, 1}}}, {{{7, 7}, {1, 1}, {5, 5}}}
    };
  return {vertices, vertexNormals, faces};
}
