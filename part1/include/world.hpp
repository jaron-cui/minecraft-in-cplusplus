#ifndef WORLD_H
#define WORLD_H
#include "scene.hpp"

/**
 *  ---------- Global Constants ----------
 */

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

/**
 *  ---------- World Representation ----------
 */

// chunks are cubic pieces of the world composed of multiple blocks
struct Chunk {
  char blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
  std::unordered_set<std::string> entityNames;

  OBJModel calculateChunkOBJ();
  static bool inBounds(glm::ivec3 localBlockCoordinate) {
    int x = localBlockCoordinate.x;
    int y = localBlockCoordinate.y;
    int z = localBlockCoordinate.z;
    return (x >=0 && x < CHUNK_SIZE) && (y >=0 && y < CHUNK_SIZE) && (z >=0 && z < CHUNK_SIZE);
  }
  char getBlock(glm::ivec3 localBlockCoordinate) {
    return blocks[localBlockCoordinate.z][localBlockCoordinate.y][localBlockCoordinate.x];
  }
  static std::string id(glm::ivec3 chunkCoordinate) {
    return std::to_string(chunkCoordinate.x) + "," + std::to_string(chunkCoordinate.y) + "," + std::to_string(chunkCoordinate.z);
  }
};

struct Hitbox {
  glm::vec3 dimensions;
};

class World;

class Entity {
  protected:
    std::string name;
    glm::vec3 position;
    glm::vec3 velocity;
    float theta;
    int health;
    Hitbox hitbox = {{1, 1, 1}};
  public:
    Entity() {}
    Entity(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity);
    // update the entity's position, behavior, etc...
    virtual void update(World &world);
    std::string getName() const;
    glm::vec3 getPosition();
    glm::vec3 getVelocity();
    void setPosition(glm::vec3 to);
    void setVelocity(glm::vec3 to);
    void accelerate(glm::vec3 acceleration);
    Hitbox getHitbox();
    virtual OBJModel getModel();
    friend class EntityGod;
};

class Player: public Entity {
  public:
    Player(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity);
    //TODO: remember that overriding doesn't work unless we pass pointers. Slicing!
    OBJModel getModel() override;
};

class World {
  protected:
    std::unordered_map<glm::ivec3, Chunk> chunks;
    std::unordered_map<std::string, Entity> entities;
  public:
    World() {
      std::cout << "Chunks: " << chunks.size() << std::endl;
      std::cout << "Entities: " << entities.size() << std::endl;
    }
    // set the chunk at specific chunk coordinates
    virtual void setChunk(glm::ivec3 chunkCoordinate, Chunk chunk);
    // return the block at specific block coordinates
    char getBlock(glm::ivec3 blockCoordinate);
    Chunk& getChunk(glm::ivec3 chunkCoordinate);
    bool hasChunk(glm::ivec3 chunkCoordinate);
    bool hasBlock(glm::ivec3 blockCoordinate);
    static glm::ivec3 blockToChunkCoordinate(glm::ivec3 blockCoordinate);
    friend class EntityGod;
    friend class TerrainGod;
};

/**
 *  ---------- The Gods ----------
 */

// gods manage the affairs of a set of chunks within a world
// there is the render god, the entity god, the terrain god
class God {
  protected:
    // the world of the god
    World &world;
    // all the chunks which this god knows of
    std::unordered_set<glm::ivec3> realm;
    // the center of the god's domain
    glm::ivec3 origin;
    // the radius of the god's domain
    int radius;
  public:
    God(World &world): world(world) {}
    // progress this god's actions
    virtual void update();
    // set this god's origin
    void setOrigin(glm::ivec3 blockCoordinate);
    // set the radius of the god's domain
    void setRadius(int chunks);
};

#endif