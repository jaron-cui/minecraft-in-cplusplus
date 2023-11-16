#ifndef WORLD_C
#define WORLD_C
#include "World.hpp"

// set the chunk at specific chunk coordinates
void World::setChunk(glm::ivec3 chunkCoordinate, Chunk chunk) {
  chunks[chunkCoordinate] = chunk;
}

// return the block at specific block coordinates
char World::getBlock(glm::ivec3 blockCoordinate) {
  glm::ivec3 chunkCoordinate = blockCoordinate / CHUNK_SIZE;
  glm::ivec3 localCoordinate = blockCoordinate - chunkCoordinate * CHUNK_SIZE;
  return chunks[chunkCoordinate].getBlock(localCoordinate);
}

Chunk& World::getChunk(glm::ivec3 chunkCoordinate) {
  return chunks[chunkCoordinate];
}

bool World::hasChunk(glm::ivec3 chunkCoordinate) {
  return chunks.find(chunkCoordinate) != chunks.end();
}
bool World::hasBlock(glm::ivec3 blockCoordinate) {
  glm::ivec3 chunkCoordinate = blockCoordinate / CHUNK_SIZE;
  return hasChunk(chunkCoordinate);
}

glm::ivec3 World::blockToChunkCoordinate(glm::ivec3 blockCoordinate) {
  return blockCoordinate / CHUNK_SIZE;
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