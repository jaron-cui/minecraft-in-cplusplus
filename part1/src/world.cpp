#ifndef WORLD_C
#define WORLD_C
#include "World.hpp"
#include <math.h>

/*
** --------- WORLD- ------
*/

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

/*
** ---------- ENTITY GOD ----------
*/

Entity::Entity(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity) {
  name = entityName;
  position = initialPosition;
  velocity = initialVelocity;
  theta = facing;
  health = -1;
}

void Entity::step(glm::vec3 impulse) {
  nextStep = impulse;
}
// tell the entity to jump during the next update
void Entity::jump() {
  jumping = true;
}

int roundDirectionally(float x, bool up) {
  return up ? std::ceil(x) : std::floor(x);
}

float roundToXPoint5(float x, bool up) {
  int sign = up ? 1 : -1;
  return roundDirectionally(x + 0.5 * sign, up) - 0.5 * sign;
}

// assumes that exactly one field is nonzero
float axisValue(glm::vec3 axis) {
  return axis.x + axis.y + axis.z;
}

// assumes axis has exactly one non-zero field
void otherAxes(glm::ivec3 axis, glm::ivec3 &r1, glm::ivec3 &r2) {
  r1 = axis.x ? POSY : POSX;
  r2 = axis.z ? POSY : POSZ;
  if (axis.x) {
    r1 = POSY;
    r2 = POSZ;
  } else if (axis.y) {
    r1 = POSZ;
    r2 = POSX;
  } else {
    r1 = POSX;
    r2 = POSY;
  }
}

float roundTieDown(float x) {
  int base = std::floor(x);
  float diff = x - base;
  return diff > 0.50001f ? base + 1 : base;
}

float roundTieUp(float x) {
  int base = std::floor(x);
  float diff = x - base;
  return diff < 0.49999f ? base : base + 1;
}

float leastIntegerGreaterThan(float x) {
  int floor = std::ceil(x);
  return floor == x ? x + 1 : floor;
}

glm::vec3 map(float (*f)(float), glm::vec3 v) {
  return glm::vec3(f(v.x), f(v.y), f(v.z));
}

// check a wall of blocks in the direction of a specific axis for collisions
bool checkDirectionForCollision(Hitbox hitbox, glm::vec3 origin, glm::vec3 velocity, glm::ivec3 axis, float travelTime, World &world) {
  glm::vec3 blockBoundaryPosition = origin + velocity * travelTime;
  // get the axes along the "wall" of blocks we might collide into
  glm::ivec3 right, up;
  otherAxes(axis, right, up);
  // define the bounds of the wall
  // the block position of the wall
  float axisVelocity = axisValue(velocity * glm::vec3(axis));
  // int axisPosition = roundDirectionally(
  //   axisValue(blockBoundaryPosition * glm::vec3(axis)),
  //   axisVelocity >= 0
  //   );
  int axisPosition = roundDirectionally(axisValue(blockBoundaryPosition * glm::vec3(axis)), axisVelocity > 0);
  int axisDirection = axisVelocity >= 0 ? 1 : -1;
  glm::vec3 hitboxTopRight = blockBoundaryPosition + hitbox.dimensions * 0.5f;
  glm::vec3 hitboxBottomLeft = blockBoundaryPosition - hitbox.dimensions * 0.5f;
  int topBound = roundTieDown(axisValue(hitboxTopRight * glm::vec3(up)));
  int rightBound = roundTieUp(axisValue(hitboxTopRight * glm::vec3(right)));
  int bottomBound = roundTieDown(axisValue(hitboxBottomLeft * glm::vec3(up)));
  int leftBound = roundTieUp(axisValue(hitboxBottomLeft * glm::vec3(right)));
  // std::cout << "checking the following ranges: [" << leftBound << ", " << rightBound << "], [" << bottomBound << ", " << topBound << "] layer " << axisPosition << std::endl;
  for (int column = bottomBound; column < topBound + 1; column += 1) {
    for (int row = leftBound; row < rightBound + 1; row += 1) {
      glm::ivec3 blockCoordinate = axisPosition * axis + column * up + row * right;
      glm::ivec3 surfaceCoordinate = blockCoordinate - axis * axisDirection;
      if (world.hasBlock(blockCoordinate) && world.getBlock(blockCoordinate) != AIR
        && (!world.hasBlock(surfaceCoordinate) || world.getBlock(surfaceCoordinate) == AIR)) {
        // std::cout << "I FOUND IT, I FOUND A SOLID BLOCK: " << blockCoordinate.x << " " << blockCoordinate.y << " " << blockCoordinate.z << " block: " << world.getBlock(blockCoordinate)<< std::endl;
        // std::cout << "H: (" << blockCoordinate.x << ", " << blockCoordinate.y << ", " << blockCoordinate.z << ") - <" << axis.x << ", " << axis.y << ", " << axis.z << ">" << std::endl;
        // std::cout << "I: L: " << axisPosition << " S[" << bottomBound << ", " << topBound << "]; T[" << leftBound << ", " << rightBound << "]" << std::endl;
        // std::cout << "O: (" << origin.x << ", " << origin.y << ", " << origin.z << ")" << std::endl;
        return true;
      }
    }
  }
  return false;
}

// calculate the times at which the vector will next cross solid block boundaries
float nextBlockCollisionTime(Hitbox hitbox, glm::vec3 origin, glm::vec3 velocity, glm::ivec3 axis, World &world) {
  float axisPosition = axisValue(origin * glm::vec3(axis));
  float axisVelocity = axisValue(velocity * glm::vec3(axis));
  if (axisVelocity == 0) {
    // there will be no intersection in this tick
    return -1;
  }
  // round the current origin position to the nearest block boundary
  // in the direction of the current velocity
  float nextGridLine = roundToXPoint5(axisPosition, axisVelocity > 0);
  float travelDistance = nextGridLine - axisPosition;
  float travelTime = travelDistance / axisVelocity;
  // check block boundaries that will be crossed in this frame for collisions
  while (travelTime <= 1.0) {
    // TODO: potentially add support for sub-block collisions with richer return results
    if (checkDirectionForCollision(hitbox, origin, velocity, axis, travelTime, world)) {
      // std::cout << "collision!" << std::endl;
      return travelTime;
    }
    // look at the next gridline
    nextGridLine += axisVelocity > 0 ? 1 : -1;
    // find how long it will take to get there in ticks
    travelDistance = nextGridLine - axisPosition;
    travelTime = travelDistance / axisVelocity;
  }
  return -1;
}

void findNextCollision(Entity &entity, World &world, float &collisionTime, glm::ivec3 &collisionAxis, glm::vec3 &reactionForce) {
  // we only need to check 3 sides of a moving box in a static world for collisions
  // float xOffset = (entity.getVelocity().x > 0 ? 1 : -1) * (entity.getHitbox().dimensions.x / 2);
  // float yOffset = (entity.getVelocity().y > 0 ? 1 : -1) * (entity.getHitbox().dimensions.y / 2);
  // float zOffset = (entity.getVelocity().z > 0 ? 1 : -1) * (entity.getHitbox().dimensions.z / 2);
  // glm::vec3 collisionSides[3] = {
  //   entity.getPosition() + xOffset * glm::vec3(POSX),
  //   entity.getPosition() + yOffset * glm::vec3(POSY),
  //   entity.getPosition() + zOffset * glm::vec3(POSZ)
  //   };

  collisionTime = 2;
  float tempTime;
  glm::ivec3 tempAxis;

  glm::vec3 v = entity.getVelocity();
  glm::vec3 directions = glm::vec3(v.x >= 0 ? 1 : -1, v.y >= 0 ? 1 : -1, v.z >= 0 ? 1 : -1);
  glm::vec3 offsets = directions * (entity.getHitbox().dimensions * 0.5f);
  
  for (glm::vec3 axis : {POSX, POSY, POSZ}) {
    glm::vec3 side = entity.getPosition() + offsets * glm::vec3(axis);
    // std::cout << offsets.x << " " << offsets.y << " " << offsets.z << std::endl;
    float projectedCollisionTime = nextBlockCollisionTime(entity.getHitbox(), side, entity.getVelocity(), axis, world);
    // std::cout << "On axis " << side.x << side.y << side.z << " collision times " <<
      // projectedCollisionTimes.x << " " << projectedCollisionTimes.y << " " << projectedCollisionTimes.z << std::endl;
    if (projectedCollisionTime >= 0 && projectedCollisionTime < collisionTime) {
      collisionTime = projectedCollisionTime;
      collisionAxis = axis;
    }
  }
  if (collisionTime <= 1) {
    reactionForce = entity.getVelocity() * -glm::vec3(collisionAxis);
  } else {
    collisionTime = -1;
  }
}

// caps a value "up to" a target. if the target is negative, this means
// the value will be unchanged if greater than but capped to if less than
float capTo(float value, float change, float cap) {
  // if (abs(value) > abs(cap)) {
  //   return value;
  // }
  if (change >= 0) {
    if (value <= cap) {
      return std::min(value + change, cap);
    } else {
      return value;
    }
  } else {
    if (value > -cap) {
      return std::max(value + change, -cap);
    } else {
      return value;
    }
  }
  // if (cap - value >= 0) {
  //   if (change < 0) {
  //     // cap >= value && change < 0
  //     return std::max(value + change, -cap);
  //   } else {
  //     // cap >= value && change >= 0
  //     return std::min(value + change, cap);
  //   }
  // } else {
  //   if (change < 0) {
  //     // cap < value && change < 0
  //     return std::max(value + change, cap);
  //   } else {
  //     // cap < value && change >= 0
  //     return std::min(value + change, -cap);
  //   }
  // }
}

// update the entity's position, behavior, etc...
void Entity::update(World &world) {
  // std::cout << "-------------------------" <<std::endl;

  // std::cout << "P: (" << position.x << ", " << position.y << ", " << position.z << ")"<< std::endl;
  // gravity
  accelerate({0, -0.006, 0});
  // player motion
  //accelerate(nextStep);
  if (axisValue(nextStep)) {
    glm::vec3 walkDirection = glm::normalize(nextStep);
    glm::vec3 cap = walkDirection * maxMovementSpeed;
    //glm::vec3 cap = velocityCap - velocity;
    velocity = glm::vec3(
      capTo(velocity.x, nextStep.x, abs(cap.x)),
      capTo(velocity.y, nextStep.y, abs(cap.y)),
      capTo(velocity.z, nextStep.z, abs(cap.z))
      );
  }

  float timeLeft = 1.0;

  // collision time is in the range [0, 1] and represents the subframe moment of collision
  float collisionTime;
  glm::ivec3 collisionAxis;
  glm::vec3 reactionForce;
  
  int limit = 10;
  while (limit > 0) {
    // getnextthing
    findNextCollision(*this, world, collisionTime, collisionAxis, reactionForce);
    // std::cout << "collision time: " << collisionTime << std::endl;
    // a collision time in the past represents no collision this frame
    if (collisionTime < 0 || collisionTime > timeLeft) {
      position += velocity * timeLeft;
      // reset impulses
      nextStep = glm::vec3(0, 0, 0);
      jumping = false;
      // std::cout << "changed position by " << velocity.y * timeLeft << std::endl;
      return;
    }
    timeLeft -= collisionTime;
    position += velocity * collisionTime;
    // TODO: maybe add bounciness or something at some point
    // std::cout << "R: <" << reactionForce.x << ", " << reactionForce.y << ", " << reactionForce.z << ">" << std::endl;
    velocity += reactionForce;

    // if there's a reaction force on the negative y axis, we can do some things
    glm::vec3 jumpForce(0);
    if (reactionForce.y > 0) {
      if (jumping) {
        jumpForce += glm::vec3(0, 0.12, 0);
      }
      // std::cout << "acceleratin'" << std::endl;
    }
    // friction
    glm::vec3 frictionForce(0);
    glm::ivec3 ortho1, ortho2;
    // removed x and z axis friction calculations because it was being weird and isn't
    // a huge priority
    for (glm::ivec3 axis : {POSY}) {
      otherAxes(axis, ortho1, ortho2);
      float s = axisValue(glm::vec3(ortho1) * velocity);
      float t = axisValue(glm::vec3(ortho2) * velocity);
      float theta = atan2(t, s);
      float r = axisValue(reactionForce * glm::vec3(axis));
      // skip friction for the head. it's weird how much it slows you down.
      if (r < 0) {
        continue;
      }
      float f = glm::min(r * 0.4f, glm::length(velocity));

      frictionForce -= glm::vec3(ortho1) * f * cos(theta);
      frictionForce -= glm::vec3(ortho2) * f * sin(theta);
      // TODO: maybe implement friction maximum
    }//std::cout << "friction: " << frictionForce.x << " " << frictionForce.y << " " << frictionForce.z << std::endl;
    accelerate(jumpForce + frictionForce);
    limit -=1;
  }
  
  if (limit == 0) {
    std::cout << "------ ERROR ------ : EXCEEDED 10 CHECKS! Last collision time: " << collisionTime << "------------------------------------------------------------------------------------------------------------" <<std::endl; 
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

void Entity::setMaxMovementSpeed(float speed) {
  maxMovementSpeed = speed;
}

Hitbox Entity::getHitbox() {
  return hitbox;
}

OBJModel Entity::getModel() {
  return UNIT_CUBE();
}
// };

Player::Player(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity):
Entity(entityName, initialPosition, facing, initialVelocity) {
  hitbox = {{12.0/16, 30.0/16, 12.0/16}};
}

OBJModel Player::getModel() {
  return Entity::getModel();
}

EntityGod::EntityGod(World &world): God(world) {};

void EntityGod::update() {
  
        // std::cout << "lock 5" << std::endl;
  world.divineIntervention.lock();
  for (auto &it : world.entities) {
    Entity &entity = it.second;
    entity.update(world);
  }
  world.divineIntervention.unlock();
  
        // std::cout << "unlock 5" << std::endl;
}

void EntityGod::createEntity(Entity entity) {
  glm::ivec3 spawnChunk = World::blockToChunkCoordinate(glm::ivec3(entity.position));
  // TODO: add name check
  
        // std::cout << "lock 6" << std::endl;
  world.divineIntervention.lock();
  world.entities[entity.name] = entity;
  if (!world.hasChunk(spawnChunk)) {
    // TODO: make a better response here
    std::cout << "CANNOT SPAWN AN ENTITY WHERE THERE IS NO CHUNK" << std::endl;
    world.divineIntervention.unlock();
    
        // std::cout << "unlock 6" << std::endl;
    return;
  }
  Chunk &chunk = world.getChunk(spawnChunk);
  chunk.entityNames.insert(entity.name);
  world.divineIntervention.unlock();
  
        // std::cout << "unlock 6 her" << std::endl;
}

struct entityNameEquals : public std::unary_function<Entity, bool> {
  explicit entityNameEquals(const std::string &name) : name(name) {}
  bool operator() (const Entity &entity) { return entity.getName() == name; }
  std::string name;
};

bool EntityGod::seesEntity(std::string name) {
  return world.entities.find(name) != world.entities.end();
}

void EntityGod::removeEntity(std::string name) {
  for (glm::ivec3 chunkCoordinate : realm) {
    world.divineIntervention.lock();
    Chunk &chunk = world.getChunk(chunkCoordinate);
    chunk.entityNames.erase(name);
    world.divineIntervention.unlock();
  }
  world.divineIntervention.lock();
  world.entities.erase(name);
  world.divineIntervention.unlock();
}

Entity& EntityGod::getEntity(std::string name) {
  return world.entities[name];
}

/*
** ----- TERRAIN  ----------------
*/

ChunkPerlinNoiseCache3D::ChunkPerlinNoiseCache3D(float noiseScale, int worldSeed, glm::ivec3 chunkCoordinates) {
  seed = worldSeed;
  scale = noiseScale;
  chunkCoordinate = chunkCoordinates;
  generateVectors();
}

ChunkPerlinNoiseCache3D::~ChunkPerlinNoiseCache3D() {
  delete [] grid;
}

glm::vec3 ChunkPerlinNoiseCache3D::blockToGridScale(glm::ivec3 blockCoordinate) const {
  return glm::vec3(blockCoordinate) / scale;
}

int ChunkPerlinNoiseCache3D::gridToIndex(glm::ivec3 gridCoordinate) const {
  glm::ivec3 offset = gridCoordinate - corner1;
  glm::ivec3 dims = corner2 - corner1 + 1;
  return dims.x * (dims.y * offset.z + offset.y) + offset.x;
}

void ChunkPerlinNoiseCache3D::generateVectors() {
  // the 2 positions in the vector grid within which
  // all chunk blocks are contained in the smallest possible volume
  corner1 = glm::ivec3(glm::floor(blockToGridScale(chunkCoordinate * CHUNK_SIZE)));
  corner2 = glm::ivec3(glm::ceil(blockToGridScale((chunkCoordinate + 1) * CHUNK_SIZE)));
  glm::ivec3 d = corner2 - corner1;
  // std::cout << "Start and end: " << startingVector.x << " " << endingVector.x << std::endl;
  std::cout << "As chunk " << chunkCoordinate.x << " " << chunkCoordinate.y <<" " << chunkCoordinate.z << std::endl;
  std::cout << "w scale " << scale << " corners: " << corner1.x << " " << corner1.y << " " << corner1.z << "; " << corner2.x << " " << corner2.y << " " << corner2.z << std::endl;
  grid = new glm::vec3[(d.z + 1) * (d.y + 1) * (d.x + 1)];
  for (int z = 0; z <= d.z; z += 1) {
    for (int y = 0; y <= d.y; y += 1) {
      for (int x = 0; x <= d.x; x += 1) {
        glm::ivec3 vectorGridCoordinate = corner1 + glm::ivec3(x, y, z);
        int vectorIndex = gridToIndex(vectorGridCoordinate);
        // std::cout << "populated: " << (d.x + 1) *((d.y + 1) * z + y) + x << std::endl;
        grid[vectorIndex] = pseudoRandomVector(vectorGridCoordinate);
        // glm::vec3 v = grid[(d.x + 1) *((d.y + 1) * z + y) + x];
        // std::cout << "I think: (" << vectorGridCoordinate.x << ", " << vectorGridCoordinate.y << ", " << vectorGridCoordinate.z << ") = <" << v.x << ", " << v.y << ", " << v.z << ">" << std::endl;
        // std::cout << "Local: (" << x << ", " << y << ", " << z << ")" << std::endl;
      }
    }
  }
}

glm::vec3 ChunkPerlinNoiseCache3D::pseudoRandomVector(glm::ivec3 vectorGridCoordinate) const {
  // reset srand
  srand(seed);
  int vectorID = vectorGridCoordinate.x * rand() + vectorGridCoordinate.y * rand() + vectorGridCoordinate.z * rand();
  // std::cout << "vector id: " << vectorID << std::endl;
  int scaleID = scale * rand() * rand();
  // std::cout << "scale id: " << scaleID << std::endl;
  // std::cout << "total id: " << vectorID * scaleID << std::endl;
  // generate a unique set of random values for a given vector of a given scale and world seed
  srand(vectorID * scaleID);
  return glm::normalize(glm::vec3((double) rand() / RAND_MAX, (double) rand() / RAND_MAX, (double) rand() / RAND_MAX) - 0.5f);
}

float ChunkPerlinNoiseCache3D::interpolate(float x, float y, float weight) const {
  if (weight < 0 || weight > 1) {
    std::cout << "OUT OF BOUNDS WEIGHT: " << weight << std::endl;
    exit(0);
  }
  return weight * weight * (3.0f - weight * 2.0f) * (y - x) + x;
}

float ChunkPerlinNoiseCache3D::sample(glm::ivec3 blockCoordinate) {
  // the block coordinate in terms of the grid (scaled down)
  glm::vec3 gridCoordinate = blockToGridScale(blockCoordinate);
  // bounding corners of the cell - vectors lie on integers
  glm::ivec3 cellOrigin = glm::ivec3(glm::floor(gridCoordinate));
  glm::ivec3 d = corner2 - corner1;
  glm::vec3 offset = gridCoordinate - glm::vec3(cellOrigin);
  if (offset.x < 0 || offset.x > 1 || offset.y < 0 || offset.y > 1 || offset.z < 0 || offset.z > 1) {
    std::cout << "PFFSET BAD: " << offset.x << " " << offset.y << " " << offset.z << std::endl;
  }
  // glm::ivec3 cornerD = cellCorner2 - cellCorner1;
  // if (cornerD.x != 1 || cornerD.y != 1 || cornerD.z != 1) {
  //   std::cout << "PFFSETss BAD: " << offset.x << " " << offset.y << " " << offset.z << std::endl;
  //   std::cout << cellCorner1.x << " " << cellCorner1.y << " " << cellCorner1.z << std::endl;
  //   std::cout << cellCorner2.x << " " << cellCorner2.y << " " << cellCorner2.z << std::endl;
  // }
  // std::cout << "Calculating dot prods..." << std::endl;

  // calculate dot products and distances
  // 0 0 0; 1 0 0; 0 1 0; 1 1 0; 0 0 1; 1 0 1; 0 1 1; 1 1 1
  // std::vector<float> distanceToCorners(8);
  std::vector<float> dotProducts;
  for (int z = 0; z <= 1; z += 1) {
    for (int y = 0; y <= 1; y += 1) {
      for (int x = 0; x <= 1; x += 1) {
        glm::ivec3 cellCorner = glm::ivec3(x, y, z) + cellOrigin;
        //distanceToCorners.push_back(glm::distance(cellCorner, gridCoordinate));
        int vectorIndex = gridToIndex(cellCorner);
        if (vectorIndex < 0 || vectorIndex >= (d.x+1) *(d.y+1)*(d.z+1) || x < 0 || y < 0 || z < 0) {
          std::cout << "OUT OF BOUNDS VECTOR!! " << vectorIndex << " " << x << " " << y << " " << z << std::endl;
        }
  // std::cout << "trying to index " << vectorIndex << " " << grid->length() << std::endl;
        // std::cout << "vector " << (d.x + 1) *((d.y + 1) * z + y) + x << ": " << grid.grid[vectorIndex].x << " " << grid.grid[vectorIndex].y << " " << grid.grid[vectorIndex].z << std::endl;
        dotProducts.push_back(glm::dot(grid[vectorIndex], gridCoordinate - glm::vec3(cellCorner)));
      }
    }
  }
  if (dotProducts.size() != 8) {
    std::cout << "DOT PRODUCTS HAS SIZE " << dotProducts.size() << std::endl;
    std::cout << gridCoordinate.x << " " << gridCoordinate.y << " " << gridCoordinate.z << std::endl;
    std::cout << corner1.x << " " << corner1.y << " " << corner1.z << std::endl;
    std::cout << cellOrigin.x << " " << cellOrigin.y << " " << cellOrigin.z << std::endl;
  }
  // std::cout << dotProducts[0] << " " << dotProducts[1] << " " << dotProducts[2] << " " << dotProducts[3] << " " << dotProducts[4] << " " << dotProducts[5] << " " << dotProducts[6] << " " << dotProducts[7] << std::endl;
  // std::cout << "offsets: " << offset.x << " " << offset.y << " " << offset.z << std::endl;
  float xi1 = interpolate(dotProducts[0], dotProducts[1], offset.x);
  float xi2 = interpolate(dotProducts[2], dotProducts[3], offset.x);
  float xi3 = interpolate(dotProducts[4], dotProducts[5], offset.x);
  float xi4 = interpolate(dotProducts[6], dotProducts[7], offset.x);
  // std::cout << "xinterpol " << xi1 << " " << xi2 << " " << xi3 << " " << xi4 << std::endl;

  float yi1 = interpolate(xi1, xi2, offset.y);
  float yi2 = interpolate(xi3, xi4, offset.y);
  // std::cout << "yinterpol " << yi1 << " " << yi2 << std::endl;

  float zi = interpolate(yi1, yi2, offset.z);
  // std::cout << "zinterpol " << zi << std::endl;

  return zi;
}

ChunkGenerator::ChunkGenerator(glm::ivec3 chunkCoord, int worldSeed, std::vector<NoiseProfile*> noiseProfiles) {
  chunkCoordinate = chunkCoord;
  seed = worldSeed;
  noises = noiseProfiles;
}

float ChunkGenerator::sampleCompoundNoise(glm::ivec3 blockCoordinate) {
  float total = 0;
  for (NoiseProfile* noise : noises) {
    total += noise->sampler.sample(blockCoordinate) * noise->magnitude;
  }
  return total;
}

Chunk ChunkGenerator::generateChunk() {
  Chunk chunk;
  // std::cout << "Generating chunk..." << std::endl;
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    // std::cout << "layer ------------------------" << std::endl;
    for (int y = 0; y < CHUNK_SIZE; y += 1) {
      for (int x = 0; x < CHUNK_SIZE; x += 1) {
        glm::ivec3 blockCoordinate = glm::ivec3(x, y, z) + chunkCoordinate * CHUNK_SIZE;
        float noiseValue = sampleCompoundNoise(blockCoordinate);
        float airThreshold = 0.6 - glm::smoothstep(-15.0f, -0.5f, float(blockCoordinate.y)) * 0.8;
        uint8_t blockType;
        if (noiseValue > airThreshold) {
          // std::cout << "  ";
          blockType = AIR;
        } else if (noiseValue > -.4) {
          // std::cout << "░░";
          blockType = 1;
        } else if (noiseValue > -0.6) {
          // std::cout << "▒▒";
          blockType = 2;
        } else if (noiseValue > -0.8) {
          // std::cout << "▓▓";
          blockType = 3;
        } else {
          // std::cout << "██";
          blockType = 4;
        }
        chunk.blocks[z][y][x] = blockType;
      }
      // std::cout << std::endl;
    }
  }
  return chunk;
}

TerrainGod::TerrainGod(World &world): God(world) {}

void TerrainGod::update() {
  std::unordered_map<glm::ivec3, glm::vec3> grid;
  std::cout << "updating terrain!" << std::endl;
  for (int z = -radius; z <= radius; z += 1) {
    for (int y = -radius; y <= radius; y += 1) {
      for (int x = -radius; x <= radius; x += 1) {
        glm::ivec3 chunkCoordinate = glm::ivec3(x, y, z);
        if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(World::blockToChunkCoordinate(origin))) > radius) {
          continue;
        }
        std::cout << "lock 1" << std::endl;
        world.divineIntervention.lock();
        if (!world.hasChunk(chunkCoordinate)) {
          world.divineIntervention.unlock();
          std::cout << "unlock 1" << std::endl;
          generateChunk(chunkCoordinate, grid);
        }
        world.divineIntervention.unlock();
        std::cout << "unlock 1 h" << std::endl;
      }
    }
  }
}

void TerrainGod::generateChunk(glm::ivec3 chunkCoordinate, std::unordered_map<glm::ivec3, glm::vec3> &grid) {
  ChunkPerlinNoiseCache3D cache1 = ChunkPerlinNoiseCache3D(40, world.seed, chunkCoordinate);
  ChunkPerlinNoiseCache3D cache2 = ChunkPerlinNoiseCache3D(9, world.seed, chunkCoordinate);
  glm::ivec3 d = cache1.corner2 - cache1.corner1;
  for (int z = 0; z <= d.z; z += 1) {
    for (int y = 0; y <= d.y; y += 1) {
      for (int x = 0; x <= d.x; x += 1) {
        glm::ivec3 gridCoordinate = glm::ivec3(x, y, z) + cache1.corner1;
        int vectorIndex = (d.x + 1) *((d.y + 1) * z + y) + x;
        if (grid.find(gridCoordinate) == grid.end()) {
          grid[gridCoordinate] = cache1.grid[vectorIndex];
        } else {
          glm::vec3 p = grid[gridCoordinate];
          glm::vec3 c = cache1.grid[vectorIndex];
          if (p != c) {
            std::cout << "I disagree! Previous at " << gridCoordinate.x << " " << gridCoordinate.y << " " << gridCoordinate.z << "=" << p.x << " " << p.y << " " << p.z << ", Current=" << c.x << " " << c.y << " " << c.z << std::endl;
          }
        }
      }
    }
  }

  NoiseProfile noise1 = {3.0f, cache1};
  NoiseProfile noise2 = {0.5f, cache2};
  std::vector<NoiseProfile*> noises{&noise1, &noise2};
  Chunk chunk = ChunkGenerator(chunkCoordinate, world.seed, noises).generateChunk();
  
        std::cout << "lock 3" << std::endl;
  world.divineIntervention.lock();
  std::cout << "Set chunk! " << std::endl;
  world.setChunk(chunkCoordinate, chunk);
  world.divineIntervention.unlock();
  
        std::cout << "unlock 3" << std::endl;
}

void TerrainGod::generateSpawn() {
  Chunk chunk = {{{0}}};
  chunk.blocks[5][14][5] = STONE;
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int x = 0; x < CHUNK_SIZE; x += 1) {
      int y = 12;//x / 3 + 9;
      chunk.blocks[z][y][x] = STONE;
    }
  }
  update();
  
        // std::cout << "lock 4" << std::endl;
  world.divineIntervention.lock();
  world.setChunk({0, -1, 0}, chunk);
  world.divineIntervention.unlock();
  
        // std::cout << "unlock 4" << std::endl;
}

/*
** ------- RENDER GOD -------------
*/


// calculate a list of faces generated selectively based on air-exposed blocks
std::vector<RenderBlockFace> calculateChunkFaces(Chunk &chunk) {
  std::vector<RenderBlockFace> faces;
  // iterate through all blocks in the chunk
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int y = 0; y < CHUNK_SIZE; y += 1) {
      for (int x = 0; x < CHUNK_SIZE; x += 1) {
        // for a given block coordinate
        glm::ivec3 blockCoordinate = {x, y, z};
        // if it is air, no faces needed
        if (chunk.getBlock(blockCoordinate) == AIR) {
          continue;
        }
        // if it is solid, then add faces for each of the air-facing sides
        for (glm::ivec3 direction : ORTHO_DIRS) {
          // don't add a face if the adjacent block is out of bounds or it is not air
          glm::ivec3 neighbor = blockCoordinate + direction;
          if (Chunk::inBounds(neighbor) && chunk.getBlock(neighbor) != AIR) {
            continue;
          }
          // add the face represented by a block coordinate and a direction
          faces.push_back({blockCoordinate, direction, chunk.getBlock(blockCoordinate)});
        }
      }
    }
  }
  return faces;
}

// TODO: instead of dynamically calculating face vertices, pull from a set of constants
// for each side for speed

// add the vertices for a given face to a vector
void addFaceVertices(OBJBuilder &builder, RenderBlockFace face) {
  glm::vec3 origin = glm::vec3(face.blockCoordinate);
  std::vector<glm::vec3> vertices(6);
  std::vector<glm::vec2> textureCoordinates(6);
  glm::vec2 blockTextureOffset = float(face.blockType - 1) * glm::vec2(1.0f, 0.0f);

  int vertexDirection = -face.facing.x | face.facing.y | -face.facing.z;
  int zero1 = face.facing.x ? face.facing.y ? 2 : 1 : 0;
  int zero2 = (face.facing.y || face.facing.x) ? 2 : 1;
  // std::cout << "zeroes: " << zero1 << ", " << zero2 << " verdir: " << vertexDirection << std::endl;
  glm::vec3 corner = {face.facing.x, face.facing.y, face.facing.z};
  // iterate through 3 indices
  for (int i = vertexDirection < 0 ? 2 : 0, _i = 0; _i < 3; i += vertexDirection, _i += 1) {
    glm::ivec2 offset1 = SQUARE_OFFSETS[TRI1[i % 3]];
    glm::ivec2 offset2 = SQUARE_OFFSETS[TRI2[i % 3]];
    corner[zero1] = offset1.x;
    corner[zero2] = offset1.y;
    // std::cout << "offset index: " << i % 3 << std::endl;
    vertices[_i] = (corner * 0.5f + origin);
    textureCoordinates[_i] = (blockTextureOffset + (glm::vec2(offset1) + 1.0f) * 0.5f) * .03125f;

    corner[zero1] = offset2.x;
    corner[zero2] = offset2.y;
    vertices[3 + _i] = (corner * 0.5f + origin);
    textureCoordinates[3 + _i] = (blockTextureOffset + (glm::vec2(offset2) + 1.0f) * 0.5f) * .03125f;
  }
  builder.addSimpleFace(vertices, textureCoordinates);
}

// TODO: optimize chunk rendering and caching by using an intermediate representation of faces that
// can be more granularly updates with changes to the chunk
OBJModel Chunk::calculateChunkOBJ() {
  OBJBuilder builder;
  for (RenderBlockFace face : calculateChunkFaces(*this)) {
    addFaceVertices(builder, face);
  }
  return builder.model;
}

RenderGod::RenderGod(World &world, Scene &openGLScene): God(world), scene(openGLScene) {}

void RenderGod::uploadCache(int max) {
  world.divineIntervention.lock();
  std::vector<glm::ivec3> uploaded;
  for (auto it : cache) {
    if (max == 0) {
      break;
    }
    scene.createMeshFromCache(Chunk::id(it.first), it.second);
    uploaded.push_back(it.first);
    max -= 1;
  }
  for (glm::ivec3 chunk : uploaded) {
    cache.erase(chunk);
  }
  world.divineIntervention.unlock();
}

void RenderGod::cullFarChunks(int allowance) {
  glm::ivec3 originChunk = World::blockToChunkCoordinate(origin);
  for (glm::ivec3 chunkCoordinate : realm) {
    if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(originChunk)) <= radius + allowance) {
      continue;
    }
    scene.deleteMesh(Chunk::id(chunkCoordinate));
    realm.erase(chunkCoordinate);
  }
}

// update the cache
void RenderGod::update() {
  int chunkCount = 0;
  glm::ivec3 originChunk = World::blockToChunkCoordinate(origin);
  for (int z = originChunk.z - radius; z < originChunk.z + radius; z += 1) {
    for (int y = originChunk.y - radius; y < originChunk.y + radius; y += 1) {
      for (int x = originChunk.x - radius; x < originChunk.x + radius; x += 1) {
        glm::ivec3 chunkCoordinate = {x, y, z};
          // std::cout << "chunk: " << chunkCoordinate.x << ", " << chunkCoordinate.y  << ", " << chunkCoordinate.z << std::endl;
        // we only care about chunks within a spherical bubble
        if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(origin) / float(CHUNK_SIZE)) > radius) {
          // std::cout << "out of chunk render sphere" << std::endl;
          continue;
        }
        // if the chunk is already cached, it must be up to date
        if (realm.find(chunkCoordinate) != realm.end()) {
          // std::cout << "chunk already cached" << std::endl;
          continue;
        }
        // TODO: if a chunk does not exist, we should generate it instead of skipping it
        // std::cout << "lock 2" << std::endl;
        world.divineIntervention.lock();
        if (!world.hasChunk(chunkCoordinate)) {
          world.divineIntervention.unlock();
          
        // std::cout << "unlock 2 here" << std::endl;
          // std::cout << "chunk does not exist" << std::endl;
          continue;
        }
        realm.insert(chunkCoordinate);
        chunkCount += 1;

          // std::cout << "rendering chunk!------------" << std::endl;
        OBJModel model = scaleOBJ(offsetOBJ(world.getChunk(chunkCoordinate).calculateChunkOBJ(), glm::vec3(chunkCoordinate * CHUNK_SIZE)), BLOCK_SCALE);
        model.vertexNormals.push_back({0, 0, 0});
        model.mtl.mapKD = "media/textures.ppm";
        world.divineIntervention.unlock();
        
        // std::cout << "unlock 2" << std::endl;

        std::vector<VBOVertex> data;
        std::vector<GLuint> indices;
        if (!encodeOBJ(model, data, indices)) {
          throw std::invalid_argument("Invalid OBJ cannot be loaded into VBO.");
        }
        cache[chunkCoordinate] = {data, indices, "media/textures.ppm"};
      }
    }
  }
  std::cout <<"Rendering THIS MANY CHUNKS: " << chunkCount << std::endl;
}

#endif