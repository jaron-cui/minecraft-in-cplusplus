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

float roundToHalf(float x, bool up) {
  return up ? std::ceil(x + 0.5) - 0.5 : std::floor(x - 0.5) + 0.5;
}

// calculate the next time a given vector intersects a block boundary
void nextVectorBlockIntersection(glm::vec3 origin, glm::vec3 velocity, float &collisionTime, glm::ivec3 &blockCoordinate) {
  collisionTime = 2.0f;
  if (velocity.x != 0) {
    float nextIntersection = roundToHalf(origin.x, velocity.x > 0);
    float delta = nextIntersection - origin.x;
    float time = delta / velocity.x;
    if (time < collisionTime) {
      collisionTime = time;
      glm::vec3 incremented = origin + velocity * time;
      blockCoordinate = {std::round(nextIntersection + velocity.x > 0 * 1 - 0.5), std::round(incremented.y), std::round(incremented.z)};
    }
  }
  if (velocity.y != 0) {
    float nextIntersection = roundToHalf(origin.y, velocity.y > 0);
    float delta = nextIntersection - origin.y;
    float time = delta / velocity.y;
    if (time < collisionTime) {
      collisionTime = time;
      glm::vec3 incremented = origin + velocity * time;
      blockCoordinate = {std::round(incremented.x), std::round(nextIntersection + velocity.y > 0 * 1 - 0.5), std::round(incremented.z)};
    }
  }
  if (velocity.z != 0) {
    float nextIntersection = roundToHalf(origin.z, velocity.z > 0);
    float delta = nextIntersection - origin.z;
    float time = delta / velocity.z;
    if (time < collisionTime) {
      collisionTime = time;
      glm::vec3 incremented = origin + velocity * time;
      blockCoordinate = {std::round(incremented.x), std::round(incremented.y), std::round(nextIntersection + velocity.z > 0 * 1 - 0.5)};
    }
  }
}

void findNextCollision(Entity &entity, World &world, float &collisionTime, glm::ivec3 &blockCoordinate, glm::vec3 &reactionForce) {
  std::vector<glm::vec3> corners(8);
  for (int i = 0; i < 2; i += 1) {
    float y = i * entity.getHitbox().height;
    for (glm::ivec2 offset : SQUARE_OFFSETS) {
      glm::vec2 squareCorner = glm::vec2(offset) * 0.5f * entity.getHitbox().width;
      corners.push_back(glm::vec3(squareCorner.x, y, squareCorner.y) + entity.getPosition());
    }
  }
  collisionTime = 2;
  float nextTime;
  glm::ivec3 nextBlock;
  for (glm::vec3 corner : corners) {
    nextVectorBlockIntersection(corner, entity.getVelocity(), nextTime, nextBlock);
    if (nextTime < collisionTime) {
      collisionTime = nextTime;
      blockCoordinate = nextBlock;
    }
  }
}

// update the entity's position, behavior, etc...
void Entity::update(World &world) {
  float timeLeft = 1.0;

  float collisionTime;
  glm::ivec3 blockCoordinate;
  glm::vec3 reactionForce;
  
  while (timeLeft > 0) {
    // getnextthing
    findNextCollision(*this, world, collisionTime, blockCoordinate, reactionForce);
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

glm::vec3 Entity::getPosition() {
  return position;
}

glm::vec3 Entity::getVelocity() {
  return velocity;
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

Hitbox Entity::getHitbox() {
  return {BLOCK_SCALE, BLOCK_SCALE};
}

OBJModel Entity::getModel() {
  return UNIT_CUBE();
}
// };

Hitbox Player::getHitbox() {
  return {BLOCK_SCALE * 0.75, BLOCK_SCALE * 1.8};
}

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
