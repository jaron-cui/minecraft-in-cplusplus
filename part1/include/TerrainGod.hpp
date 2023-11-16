#include "World.hpp"

class TerrainGod: public God {
  public:
    TerrainGod(World &world);
    void generateSpawn();
};
