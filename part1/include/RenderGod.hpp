#include "World.hpp"

class RenderGod: public God {
  private:
    Scene &scene;
  public:
    RenderGod(World &world, Scene &scene);
    void setChunk(glm::ivec3 chunkCoordinate, Chunk chunk);
    // update the cache
    void update() override;
};

struct RenderBlockFace {
  glm::ivec3 blockCoordinate;
  glm::ivec3 facing;
};

// TODO: remove
void addFaceVertices(OBJBuilder* builder, RenderBlockFace face);