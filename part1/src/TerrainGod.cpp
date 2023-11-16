#include "TerrainGod.hpp"

TerrainGod::TerrainGod(World &world): God(world) {}

void TerrainGod::generateSpawn() {
  Chunk chunk = {{{0}}};
  chunk.blocks[5][14][5] = STONE;
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int x = 0; x < CHUNK_SIZE; x += 1) {
      chunk.blocks[z][14][x] = STONE;
    }
  }
  int rad = 4;
  for (int z = -rad; z <= rad; z += 1) {
    for (int x = -rad; x <= rad; x += 1) {
      world.setChunk({x, -1, z}, chunk);
    }
  }
}