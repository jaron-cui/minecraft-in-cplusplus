#include "world.hpp"
#include <stdexcept>

// calculate a list of faces generated selectively based on air-exposed blocks
std::vector<RenderBlockFace> calculateChunkFaces(Chunk &chunk) {
  std::vector<RenderBlockFace> faces;
  // iterate through all blocks in the chunk
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int y = 0; y < CHUNK_SIZE; y += 1) {
      for (int x = 0; x < CHUNK_SIZE; x += 1) {
        // for a given block coordinate
        glm::ivec3 blockCoordinate = {x, y, z};
        // if it is air, no faces needed
        if (chunk.getBlock(blockCoordinate) == AIR) {
          continue;
        }
        // if it is solid, then add faces for each of the air-facing sides
        for (glm::ivec3 direction : ORTHO_DIRS) {
          // don't add a face if the adjacent block is out of bounds or it is air
          glm::ivec3 neighbor = blockCoordinate + direction;
          if (!Chunk::inBounds(neighbor) || chunk.getBlock(neighbor) == AIR) {
            continue;
          }
          // add the face represented by a block coordinate and a direction
          faces.push_back({blockCoordinate, direction});
        }
      }
    }
  }
  return faces;
}

// TODO: instead of dynamically calculating face vertices, pull from a set of constants
// for each side for speed

// add the vertices for a given face to a vector
void addFaceVertices(OBJBuilder &builder, RenderBlockFace face) {
  glm::vec3 origin = glm::vec3(face.blockCoordinate);
  std::vector<glm::vec3> vertices(6);
  int vertexDirection = face.facing.x | face.facing.y | face.facing.z;
  int zero1 = face.facing.x ? face.facing.y ? 2 : 1 : 0;
  int zero2 = (face.facing.y || face.facing.x) ? 2 : 1;
  std::cout << "zeroes: " << zero1 << ", " << zero2 << " verdir: " << vertexDirection << std::endl;
  int corner[3] = {face.facing.x, face.facing.y, face.facing.z};
  // iterate through 3 indices
  for (int i = 0, _i = 0; _i < 3; i += vertexDirection, _i += 1) {
    glm::ivec2 offset1 = SQUARE_OFFSETS[TRI1[i % 3]];
    glm::ivec2 offset2 = SQUARE_OFFSETS[TRI2[i % 3]];
    corner[zero1] = offset1.x;
    corner[zero2] = offset1.y;
    std::cout << "offset index: " << i % 3 << std::endl;
    vertices[_i] = (glm::vec3(corner[0], corner[1], corner[2]) * 0.5f + origin) * BLOCK_SCALE;
    corner[zero1] = offset2.x;
    corner[zero2] = offset2.y;
    vertices[3 + _i] = (glm::vec3(corner[0], corner[1], corner[2]) * 0.5f + origin) * BLOCK_SCALE;
  }
  builder.addSimpleFace(vertices);
}

// TODO: optimize chunk rendering and caching by using an intermediate representation of faces that
// can be more granularly updates with changes to the chunk
OBJModel Chunk::calculateChunkVertices() {
  OBJBuilder builder;
  std::vector<VBOVertex> vertices;
  for (RenderBlockFace face : calculateChunkFaces(*this)) {
    addFaceVertices(builder, face);
  }
  return builder.model;
}

// set the chunk at specific chunk coordinates
void World::setChunk(glm::ivec3 chunkCoordinate, Chunk chunk) {

}
// return the block at specific block coordinates
char World::getBlock(glm::ivec3 blockCoordinate) {
  return 0;
}
bool World::hasChunk(glm::ivec3 chunkCoordinate) {
return 0;
}
bool World::hasBlock(glm::ivec3 blockCoordinate) {
return 0;
}

// set the render origin to the specified block coordinates
// this is the center of the render sphere
void RenderWorld::setRenderOrigin(glm::ivec3 blockCoordinate) {

}
// set the radius of the render sphere, in chunks
void RenderWorld::setRenderRadius(int chunks) {

}
// get the cache of vertices to render
std::vector<VBOVertex> RenderWorld::getVertexCache() {
  std::vector<VBOVertex> vertices;
  return vertices;
}
// update the cache
void RenderWorld::updateRenderCache() {

}
