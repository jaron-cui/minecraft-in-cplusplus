#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>
#include "scene.hpp"

// chunk coordinates * CHUNK_SIZE = block coordinates of the (0, 0, 0) corner of the chunk
const int CHUNK_SIZE = 16;

const float BLOCK_SCALE = 0.5;

const char AIR = 0;
const char STONE = 1;

// represents orthogonal directions along the x, y, or z axis
const glm::ivec3 POSX = {1, 0, 0};
const glm::ivec3 NEGX = {-1, 0, 0};
const glm::ivec3 POSY = {0, 1, 0};
const glm::ivec3 NEGY = {0, -1, 0};
const glm::ivec3 POSZ = {0, 0, 1};
const glm::ivec3 NEGZ = {0, 0, -1};
const glm::ivec3 ORTHO_DIRS[6] = {POSX, NEGX, POSY, NEGY, POSZ, NEGZ};

const glm::ivec2 SQUARE_OFFSETS[4] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
const int TRI1[3] = {0, 1, 2};
const int TRI2[3] = {1, 3, 2};

// chunks are cubic pieces of the world composed of multiple blocks
struct Chunk {
  char blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  OBJModel calculateChunkVertices();
  static bool inBounds(glm::ivec3 localBlockCoordinate) {
    int x = localBlockCoordinate.x;
    int y = localBlockCoordinate.y;
    int z = localBlockCoordinate.z;
    return (x >=0 && x < CHUNK_SIZE) && (y >=0 && y < CHUNK_SIZE) && (z >=0 && z < CHUNK_SIZE);
  }
  char getBlock(glm::ivec3 localBlockCoordinate) {
    return blocks[localBlockCoordinate.z][localBlockCoordinate.y][localBlockCoordinate.x];
  }
};

class World {
  private:
    std::unordered_map<glm::ivec3, Chunk> chunks;
  public:
    // set the chunk at specific chunk coordinates
    void setChunk(glm::ivec3 chunkCoordinate, Chunk chunk);
    // return the block at specific block coordinates
    char getBlock(glm::ivec3 blockCoordinate);
    bool hasChunk(glm::ivec3 chunkCoordinate);
    bool hasBlock(glm::ivec3 blockCoordinate);
};

class RenderWorld: public World {
  private:
    std::unordered_map<glm::ivec3, std::vector<VBOVertex>> chunkVertexCache;
    std::vector<VBOVertex> worldVertexCache;
    glm::ivec3 renderOrigin;
  public:
    // set the render origin to the specified block coordinates
    // this is the center of the render sphere
    void setRenderOrigin(glm::ivec3 blockCoordinate);
    // set the radius of the render sphere, in chunks
    void setRenderRadius(int chunks);
    // get the cache of vertices to render
    std::vector<VBOVertex> getVertexCache();
    // update the cache
    void updateRenderCache();
};

struct RenderBlockFace {
  glm::ivec3 blockCoordinate;
  glm::ivec3 facing;
};

// TODO: remove
void addFaceVertices(OBJBuilder* builder, RenderBlockFace face);
