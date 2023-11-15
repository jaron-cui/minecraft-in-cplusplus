#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>
#include "scene.hpp"

// chunk coordinates * CHUNK_SIZE = block coordinates of the (0, 0, 0) corner of the chunk
const int CHUNK_SIZE = 16;

struct Chunk {
  char blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  std::vector<VBOVertex> calculateChunkVertices();
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