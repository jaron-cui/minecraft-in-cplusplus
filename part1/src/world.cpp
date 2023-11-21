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
      if (!world.hasBlock(blockCoordinate)) {
        return true;
      }
      if (world.hasBlock(blockCoordinate) && world.getBlock(blockCoordinate) != BLOCKTYPE_AIR
        && (!world.hasBlock(surfaceCoordinate) || world.getBlock(surfaceCoordinate) == BLOCKTYPE_AIR)) {
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
  // std::cout << "As chunk " << chunkCoordinate.x << " " << chunkCoordinate.y <<" " << chunkCoordinate.z << std::endl;
  // std::cout << "w scale " << scale << " corners: " << corner1.x << " " << corner1.y << " " << corner1.z << "; " << corner2.x << " " << corner2.y << " " << corner2.z << std::endl;
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
  int vectorID = vectorGridCoordinate.x * rand() ^ vectorGridCoordinate.y * rand() ^ vectorGridCoordinate.z * rand();
  int scaleID = scale * rand();
  srand(vectorID ^ scaleID);
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

  std::vector<float> dotProducts;
  for (int z = 0; z <= 1; z += 1) {
    for (int y = 0; y <= 1; y += 1) {
      for (int x = 0; x <= 1; x += 1) {
        glm::ivec3 cellCorner = glm::ivec3(x, y, z) + cellOrigin;
        //distanceToCorners.push_back(glm::distance(cellCorner, gridCoordinate));
        int vectorIndex = gridToIndex(cellCorner);
  // std::cout << "trying to index " << vectorIndex << " " << grid->length() << std::endl;
        // std::cout << "vector " << (d.x + 1) *((d.y + 1) * z + y) + x << ": " << grid.grid[vectorIndex].x << " " << grid.grid[vectorIndex].y << " " << grid.grid[vectorIndex].z << std::endl;
        dotProducts.push_back(glm::dot(grid[vectorIndex], gridCoordinate - glm::vec3(cellCorner)));
      }
    }
  }
  // std::cout << dotProducts[0] << " " << dotProducts[1] << " " << dotProducts[2] << " " << dotProducts[3] << " " << dotProducts[4] << " " << dotProducts[5] << " " << dotProducts[6] << " " << dotProducts[7] << std::endl;
  // std::cout << "offsets: " << offset.x << " " << offset.y << " " << offset.z << std::endl;
  float xi1 = interpolate(dotProducts[0], dotProducts[2], offset.y);
  float xi2 = interpolate(dotProducts[1], dotProducts[3], offset.y);
  float xi3 = interpolate(dotProducts[4], dotProducts[6], offset.y);
  float xi4 = interpolate(dotProducts[5], dotProducts[7], offset.y);
  // std::cout << "xinterpol " << xi1 << " " << xi2 << " " << xi3 << " " << xi4 << std::endl;

  float yi1 = interpolate(xi1, xi2, offset.x);
  float yi2 = interpolate(xi3, xi4, offset.x);
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
  // TODO:
  // 1. Larger context noise cache for inter-chunkiness
  // 2. Perhaps a chunk baking range for polishing once further chunks are generated
  //  - Lets us make cross-chunk structures like trees and buildings
  //  - Lets us check for air exposure, add detail like grass and... mobs?
  // 3. 2D perlin noise for:
  //  - Ground level
  //  - Ruggedness
  //  - Biome
  //    > Temperature
  //    > Humidity
  // 4. Random noise for:
  //  - 3D "seeding" blocks - where we can begin generating
  //         small structures like trees, ores, pumpkins
  // (idea?) 5. Random per-chunk value where if above a certain threshold, finds
  //  - all connected chunks above the threshold and generates a large-scale
  //    structure if there are enough connected chunks to fit it?
  // (idea?) 6. Maybe a better idea - some way to find random points isometrically
  //  - within a certain range
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    // std::cout << "layer ------------------------" << std::endl;
    for (int y = 0; y < CHUNK_SIZE; y += 1) {
      for (int x = 0; x < CHUNK_SIZE; x += 1) {
        glm::ivec3 blockCoordinate = glm::ivec3(x, y, z) + chunkCoordinate * CHUNK_SIZE;
        float noiseValue = sampleCompoundNoise(blockCoordinate);
        float seaLevel = -10;
        float groundLevel = -6;
        float ruggedNess = 16;
        float airiness = glm::smoothstep(groundLevel - ruggedNess, groundLevel + ruggedNess, float(blockCoordinate.y));
        float dirtDepth = groundLevel - ruggedNess;
        // underground will be 50% air, aboveground will be 80% air
        float airThreshold = -0.1 + airiness * 0.7;
        float dirtThreshold = airThreshold + 0.2;
        uint8_t blockType;
        if (noiseValue < airThreshold) {
          // std::cout << "  ";
          blockType = BLOCKTYPE_AIR;
        } else if (noiseValue >= airThreshold && noiseValue < dirtThreshold && blockCoordinate.y >= dirtDepth) {
          // std::cout << "░░";
          blockType = BLOCKTYPE_DIRT;
        } else {
          blockType = BLOCKTYPE_STONE;
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
  // std::cout << "updating terrain!" << std::endl;
  glm::ivec3 lo = World::blockToChunkCoordinate(origin) - radius;
  glm::ivec3 hi = World::blockToChunkCoordinate(origin) + radius;
  for (int z = lo.z; z <= hi.z; z += 1) {
    for (int y = lo.y; y <= hi.y; y += 1) {
      for (int x = lo.x; x <= hi.x; x += 1) {
        glm::ivec3 chunkCoordinate = glm::ivec3(x, y, z);
        if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(World::blockToChunkCoordinate(origin))) > radius) {
          continue;
        }
        world.divineIntervention.lock();
        if (!world.hasChunk(chunkCoordinate)) {
          world.divineIntervention.unlock();
          generateChunk(chunkCoordinate, grid);
        }
        world.divineIntervention.unlock();
      }
    }
  }
}

void TerrainGod::generateChunk(glm::ivec3 chunkCoordinate, std::unordered_map<glm::ivec3, glm::vec3> &grid) {
  ChunkPerlinNoiseCache3D cache1 = ChunkPerlinNoiseCache3D(40, world.seed, chunkCoordinate);
  ChunkPerlinNoiseCache3D cache2 = ChunkPerlinNoiseCache3D(9, world.seed, chunkCoordinate);
  glm::ivec3 d = cache1.corner2 - cache1.corner1;

  NoiseProfile noise1 = {0.7f, cache1};
  NoiseProfile noise2 = {0.3f, cache2};
  std::vector<NoiseProfile*> noises{&noise1, &noise2};
  Chunk chunk = ChunkGenerator(chunkCoordinate, world.seed, noises).generateChunk();
  
  world.divineIntervention.lock();
  world.setChunk(chunkCoordinate, chunk);
  world.divineIntervention.unlock();
}

void TerrainGod::generateSpawn() {
  Chunk chunk = {{{0}}};
  chunk.blocks[5][14][5] = BLOCKTYPE_LEAVES;
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int x = 0; x < CHUNK_SIZE; x += 1) {
      int y = 12;//x / 3 + 9;
      chunk.blocks[z][y][x] = BLOCKTYPE_BRICK;
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
        if (chunk.getBlock(blockCoordinate) == BLOCKTYPE_AIR) {
          continue;
        }
        // if it is solid, then add faces for each of the air-facing sides
        for (glm::ivec3 direction : ORTHO_DIRS) {
          // don't add a face if the adjacent block is out of bounds or it is not air
          glm::ivec3 neighbor = blockCoordinate + direction;
          if (Chunk::inBounds(neighbor) && chunk.getBlock(neighbor) != BLOCKTYPE_AIR) {
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
  std::vector<glm::vec3> normals(6);
  std::vector<glm::vec2> textureCoordinates(6);
  glm::vec2 blockTextureOffset = float(face.blockType - 1) * glm::vec2(1.0f, 0.0f);

  int vertexDirection = -(face.facing.x | face.facing.y | face.facing.z);
  glm::ivec3 axis1, axis2;
  otherAxes(face.facing, axis1, axis2);
  // std::cout << "zeroes: " << zero1 << ", " << zero2 << " verdir: " << vertexDirection << std::endl;
  glm::vec3 corner = {face.facing.x, face.facing.y, face.facing.z};
  // iterate through 3 indices
  for (int i = vertexDirection < 0 ? 2 : 0, _i = 0; _i < 3; i += vertexDirection, _i += 1) {
    glm::ivec2 offset1 = SQUARE_OFFSETS[TRI1[i % 3]];
    glm::ivec2 offset2 = SQUARE_OFFSETS[TRI2[i % 3]];
    glm::vec3 corner1 = glm::vec3(face.facing + axis1 * offset1.x + axis2 * offset1.y);
    vertices[_i] = origin + 0.5f * corner1;
    //normals[_i] = glm::normalize(corner1);
    normals[_i] = face.facing;
    textureCoordinates[_i] = (blockTextureOffset + (glm::vec2(offset1) + 1.0f) * 0.5f) * .03125f;

    glm::vec3 corner2 = glm::vec3(face.facing + axis1 * offset2.x + axis2 * offset2.y);
    vertices[_i + 3] = origin + 0.5f * corner2;
    //normals[_i + 3] = glm::normalize(corner2);
    normals[_i + 3] = face.facing;
    textureCoordinates[_i + 3] = (blockTextureOffset + (glm::vec2(offset2) + 1.0f) * 0.5f) * .03125f;
  }
  builder.addSimpleFace(vertices, normals, textureCoordinates);
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

RenderGod::RenderGod(World &world, Scene &openGLScene): God(world), scene(openGLScene) {
  scene.createSun("sun", {
    glm::vec3()
  });
}

void RenderGod::updateSun() {
  // each hour is 60
  const int hour = 60;
  const int dayCycle = 24 * hour;
  int cycleTime = world.time % dayCycle;
  // 12:00 PM
  int noon = 12 * hour;
  // 20 hours of daylight
  int daylightDuration = 16 * hour;
  int sunrise = noon - daylightDuration / 2;
  int sunset = noon + daylightDuration / 2;
  // the pretty colors in the sky during sunrise and sunset
  // will last 1 hour
  int nightTransitionDuration = 5 * hour;
  float sunriseProgress = glm::smoothstep(
    float(sunrise - nightTransitionDuration / 2),
    float(sunrise + nightTransitionDuration / 2),
    float(cycleTime)
  );
  float sunsetProgress = glm::smoothstep(
    float(sunset - nightTransitionDuration / 2),
    float(sunset + nightTransitionDuration / 2),
    float(cycleTime)
  );
  // how blue the sky should be
  float transitionProgress = sunriseProgress - sunsetProgress;
  // TODO: we could make the sky not just a single color,
  //       where the side of the sky where the sun is is more colored
  glm::vec3 nightSky = glm::vec3(4, 7, 13) / 256.0f;
  glm::vec3 nightLight = glm::vec3(0, 0, 0);// glm::vec3(87, 80, 107) / 256.0f;
  glm::vec3 transitionSky = glm::vec3(255, 155, 48) / 256.0f * 0.6f;
  glm::vec3 transitionLight = glm::vec3(255, 211, 150) / 256.0f * 0.6f;
  glm::vec3 daySky = glm::vec3(117, 156, 235) / 256.0f;
  glm::vec3 noonSky = glm::vec3(176, 202, 255) / 256.0f;
  glm::vec3 dayLight = glm::vec3(1, 1, 1) * 1.0f;
  float transitionRad = (transitionProgress - 0.5f) * 2.0f;
  glm::vec3 skyColor;
  glm::vec3 lightColor;
  if (transitionRad < 0.0f) {
    skyColor = -transitionRad * nightSky + (transitionRad + 1) * transitionSky;
    lightColor = -transitionRad * nightLight + (transitionRad + 1) * transitionLight;
  } else {
    float noonness = 1 - float(abs(noon - cycleTime)) / daylightDuration * 2.0f;
    skyColor = transitionRad * ((1 - noonness) * daySky + noonness * noonSky) + (1 - transitionRad) * transitionSky;
    lightColor = transitionRad * (dayLight * (noonness * 0.2f + 0.8f)) + (1 - transitionRad) * transitionLight;
  }
  // std::cout << "trans progress: " << transitionProgress << " transitionRad: " << transitionRad << std::endl;
  std::cout << "day cycle: " << cycleTime << std::endl;
  int nightTimeDuration = dayCycle - daylightDuration;
  float degrees;
  if (cycleTime < sunrise) {
    float slope = 180.0f / nightTimeDuration;
    float intercept = (dayCycle - sunset) * slope + 180.0f;
    degrees = slope * cycleTime + intercept;
  } else if (cycleTime < dayCycle && cycleTime >= sunset) {
    float slope = 180.0f / nightTimeDuration;
    degrees = slope * (cycleTime - sunset) + 180.0f;
  } else {
    float slope = 180.0f / daylightDuration;
    degrees = (cycleTime - sunrise) * slope;
  }
  float rads = glm::radians(degrees);
  // 0.2f is a little extra tilt
  glm::vec3 sunDirection = glm::normalize(glm::vec3(std::cos(rads), std::sin(rads), 0.2f));
  // TODO: set sun visual somehow
  scene.getSun("sun")->color = lightColor;
  scene.getSun("sun")->direction = sunDirection;
  scene.setBackground(skyColor);
}

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

void RenderGod::cullFarChunks(int allowance, int max) {
  glm::ivec3 originChunk = World::blockToChunkCoordinate(origin);
  for (glm::ivec3 chunkCoordinate : realm) {
    if (max == 0) {
      break;
    }
    if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(originChunk)) <= radius + allowance) {
      continue;
    }
    scene.deleteMesh(Chunk::id(chunkCoordinate));
    realm.erase(chunkCoordinate);
    max -= 1;
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