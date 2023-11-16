#include "EntityGod.hpp"

// gods manage the affairs of a set of chunks within a world
// there is the render god, the entity god, the terrain god
// class God {
//   protected:
//     // all the chunks which this god knows of
//     std::unordered_set<glm::ivec3> realm;
//     // the center of the god's domain
//     glm::ivec3 origin;
//     // the radius of the god's domain
//     int radius;
//   public:
//     // progress this god's actions
//     void update();
// };

// class Entity {
//   protected:
//     std::string name;
//     glm::vec3 position;
//     glm::vec3 velocity;
//     float theta;
//     int health;
//   public:
Entity::Entity(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity) {
  name = entityName;
  position = initialPosition;
  velocity = initialVelocity;
  theta = facing;
  health = -1;
}
// update the entity's position, behavior, etc...
void Entity::update(World &world) {
  float timeLeft = 1.0;

  float collisionTime;
  glm::ivec3 blockCoordinate;
  glm::vec3 reactionForce;
  
  while (timeLeft > 0) {
    // getnextthing
    findNextCollision(*this, world);
    if (collisionTime > timeLeft) {
      return;
    }
    position += velocity * collisionTime;
    // TODO: maybe add bounciness or something at some point
    velocity += reactionForce;
  }
}

std::string Entity::getName() const {
  return name;
}

void Entity::setPosition(glm::vec3 to) {
  position = to;
}

void Entity::setVelocity(glm::vec3 to) {
  velocity = to;
}

void Entity::accelerate(glm::vec3 acceleration) {
  velocity += acceleration;
}

OBJModel Entity::getModel() {
  return UNIT_CUBE();
}
// };

OBJModel Player::getModel() {
  return Entity::getModel();
}

void EntityGod::createEntity(Entity entity) {
  Chunk &chunk = world.getChunk(World::blockToChunkCoordinate(entity.position));
  chunk.entities.insert(entity);
}

struct entityNameEquals : public std::unary_function<Entity, bool> {
  explicit entityNameEquals(const std::string &name) : name(name) {}
  bool operator() (const Entity &entity) { return entity.getName() == name; }
  std::string name;
};

bool EntityGod::seesEntity(std::string name) {
  for (glm::ivec3 chunkCoordinate : realm) {
    Chunk &chunk = world.getChunk(chunkCoordinate);
    auto it = std::find_if(chunk.entities.begin(), chunk.entities.end(), entityNameEquals(name));
    if (it != chunk.entities.end()) {
      return true;
    }
  }
  return false;
}

void EntityGod::removeEntity(std::string name) {
  for (glm::ivec3 chunkCoordinate : realm) {
    Chunk &chunk = world.getChunk(chunkCoordinate);
    std::remove_if(chunk.entities.begin(), chunk.entities.end(), entityNameEquals(name));
  }
}
