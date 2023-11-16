#ifndef WORLD_C
#define WORLD_C
#include "World.hpp"

// set the chunk at specific chunk coordinates
void World::setChunk(glm::ivec3 chunkCoordinate, Chunk chunk) {
  chunks[chunkCoordinate] = chunk;
}

// return the block at specific block coordinates
char World::getBlock(glm::ivec3 blockCoordinate) {
  glm::ivec3 chunkCoordinate = World::blockToChunkCoordinate(blockCoordinate);
  glm::ivec3 localCoordinate = blockCoordinate - chunkCoordinate * CHUNK_SIZE;
  // if (!hasChunk(chunkCoordinate)) {
  //   std::cout << "CHUNK DOES NOT EXIST" << std::endl;
  // }
  return getChunk(chunkCoordinate).getBlock(localCoordinate);
}

Chunk& World::getChunk(glm::ivec3 chunkCoordinate) {
  return chunks[chunkCoordinate];
}

bool World::hasChunk(glm::ivec3 chunkCoordinate) {
  return chunks.find(chunkCoordinate) != chunks.end();
}
bool World::hasBlock(glm::ivec3 blockCoordinate) {
  glm::ivec3 chunkCoordinate = World::blockToChunkCoordinate(blockCoordinate);
  return hasChunk(chunkCoordinate);
}

glm::ivec3 World::blockToChunkCoordinate(glm::ivec3 blockCoordinate) {
  return glm::ivec3(glm::floor(glm::vec3(blockCoordinate) / float(CHUNK_SIZE)));
}

void God::setOrigin(glm::ivec3 blockCoordinate) {
  origin = blockCoordinate;
}

// set the radius of the god's domain
void God::setRadius(int chunks) {
  radius = chunks;
}

void God::update() {}

#endif