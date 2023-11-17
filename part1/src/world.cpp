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

float roundToHalf(float x, bool up) {
  return roundDirectionally(x + 0.5, up) - 0.5;
}

// assumes that exactly one field is nonzero
float axisValue(glm::vec3 axis) {
  return axis.x + axis.y + axis.z;
}

// assumes axis has exactly one non-zero field
void otherAxes(glm::ivec3 axis, glm::ivec3 &r1, glm::ivec3 &r2) {
  r1 = axis.x ? POSY : POSX;
  r2 = axis.z ? POSY : POSZ;
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
  int axisPosition = roundDirectionally(
    axisValue(blockBoundaryPosition * glm::vec3(axis)),
    axisVelocity >= 0
    );
  int axisDirection = axisVelocity >= 0 ? 1 : -1;
  glm::vec3 hitboxTopRight = blockBoundaryPosition + hitbox.dimensions / 2.0f;
  glm::vec3 hitboxBottomLeft = blockBoundaryPosition - hitbox.dimensions / 2.0f;
  int topBound = std::ceil(axisValue(hitboxTopRight * glm::vec3(up)));
  int rightBound = std::ceil(axisValue(hitboxTopRight * glm::vec3(right)));
  int bottomBound = std::floor(axisValue(hitboxBottomLeft * glm::vec3(up)));
  int leftBound = std::floor(axisValue(hitboxBottomLeft * glm::vec3(right)));
  // std::cout << "checking the following ranges: [" << leftBound << ", " << rightBound << "], [" << bottomBound << ", " << topBound << "] layer " << axisPosition << std::endl;
  for (int column = bottomBound; column < topBound + 1; column += 1) {
    for (int row = leftBound; row < rightBound + 1; row += 1) {
      glm::ivec3 blockCoordinate = axisPosition * axis + column * up + row * right;
      glm::ivec3 surfaceCoordinate = blockCoordinate - axis * axisDirection;
      if (world.hasBlock(blockCoordinate) && world.getBlock(blockCoordinate) != AIR
        && world.hasBlock(surfaceCoordinate) && world.getBlock(surfaceCoordinate) == AIR) {
        // std::cout << "I FOUND IT, I FOUND A SOLID BLOCK: " << blockCoordinate.x << " " << blockCoordinate.y << " " << blockCoordinate.z << " block: " << world.getBlock(blockCoordinate)<< std::endl;
        std::cout << "H: (" << blockCoordinate.x << ", " << blockCoordinate.y << ", " << blockCoordinate.z << ") - <" << axis.x << ", " << axis.y << ", " << axis.z << ">" << std::endl;
        return true;
      } else {
  glm::ivec3 chunkCoordinate = World::blockToChunkCoordinate(blockCoordinate);
  glm::ivec3 localCoordinate = blockCoordinate - chunkCoordinate * CHUNK_SIZE;
  // std::cout << "chunk coordinate: " << chunkCoordinate.x << " " << chunkCoordinate.y << " " << chunkCoordinate.z << " local: " << localCoordinate.x << " " << localCoordinate.y << " " << localCoordinate.z << std::endl;
        // std::cout << "Block coordinate " << blockCoordinate.x << " " << blockCoordinate.y << " " << blockCoordinate.z << " is " << (world.hasBlock(blockCoordinate) && world.getBlock(blockCoordinate)) << std::endl;
        // for (int z = 0; z < CHUNK_SIZE; z += 1) {
        //   for (int y = 0; y < CHUNK_SIZE; y += 1) {
        //   for (int x = 0; x < CHUNK_SIZE; x += 1) {
        //   std::cout << int(world.getChunk(chunkCoordinate).getBlock({x, y, z})) << " ";
        // }
        // std::cout << std::endl;
        // }
        // std::cout << std::endl;
        // }
        // glm::ivec3 bc = {0, -2, 0};
        // glm::ivec3 cc = World::blockToChunkCoordinate(bc);
        // glm::ivec3 lc = bc - cc * CHUNK_SIZE;
        // std::cout << "Has block? " << world.hasBlock(bc) << std::endl;
        // std::cout << "BLOCK COORDINATE 0 -2 0: " << int(world.getChunk(cc).getBlock(lc)) << " " << int(world.getBlock(bc)) << std::endl;
      }
    }
  }
  return false;
}

// calculate the times at which the vector will next cross solid block boundaries
glm::vec3 nextBlockCollisionTimes(Hitbox hitbox, glm::vec3 origin, glm::vec3 velocity, World &world) {
  glm::vec3 collisionTimes = {-1, -1, -1};

  for (glm::ivec3 axis : {POSX, POSY, POSZ}) {
    float axisPosition = axisValue(origin * glm::vec3(axis));
    float axisVelocity = axisValue(velocity * glm::vec3(axis));
    // no block boundary intersection if no velocity component
    if (axisVelocity == 0) {
      // there will be no intersection in this tick
      continue;
    }
    // round the current origin position to the nearest block boundary
    // in the direction of the current velocity
    float nextGridLine = roundToHalf(axisPosition, axisVelocity > 0);
    float travelDistance = nextGridLine - axisPosition;
    float travelTime = travelDistance / axisVelocity;
    // check block boundaries that will be crossed in this frame for collisions
    while (travelTime <= 1.0) {
      // TODO: potentially add support for sub-block collisions with richer return results
      if (checkDirectionForCollision(hitbox, origin, velocity, axis, travelTime, world)) {
        collisionTimes += (travelTime + 1) * glm::vec3(axis);
        break;
      }
      // look at the next gridline
      nextGridLine += axisVelocity > 0 ? 1 : -1;
      // find how long it will take to get there in ticks
      travelDistance = nextGridLine - axisPosition;
      travelTime = travelDistance / axisVelocity;
    }
  }
  return collisionTimes;
}

void findNextCollision(Entity &entity, World &world, float &collisionTime, glm::ivec3 &collisionAxis, glm::vec3 &reactionForce) {
  // we only need to check 3 sides of a moving box in a static world for collisions
  float xOffset = (entity.getVelocity().x > 0 ? 1 : -1) * (entity.getHitbox().dimensions.x / 2);
  float yOffset = (entity.getVelocity().y > 0 ? 1 : -1) * (entity.getHitbox().dimensions.y / 2);
  float zOffset = (entity.getVelocity().z > 0 ? 1 : -1) * (entity.getHitbox().dimensions.z / 2);
  glm::vec3 collisionSides[3] = {
    entity.getPosition() + xOffset * glm::vec3(POSX),
    entity.getPosition() + yOffset * glm::vec3(POSY),
    entity.getPosition() + zOffset * glm::vec3(POSZ)
    };

  collisionTime = 2;
  float tempTime;
  glm::ivec3 tempAxis;
  for (glm::vec3 side : collisionSides) {
    glm::vec3 projectedCollisionTimes = nextBlockCollisionTimes(entity.getHitbox(), side, entity.getVelocity(), world);
    // std::cout << "On axis " << side.x << side.y << side.z << " collision times " <<
      // projectedCollisionTimes.x << " " << projectedCollisionTimes.y << " " << projectedCollisionTimes.z << std::endl;
    if (projectedCollisionTimes.x >= 0 && projectedCollisionTimes.x < collisionTime) {
      collisionTime = projectedCollisionTimes.x;
      // the x axis, could be negative x as well
      collisionAxis = POSX;
    }
    if (projectedCollisionTimes.y >= 0 && projectedCollisionTimes.y < collisionTime) {
      collisionTime = projectedCollisionTimes.y;
      collisionAxis = POSY;
    }
    if (projectedCollisionTimes.z >= 0 && projectedCollisionTimes.z < collisionTime) {
      collisionTime = projectedCollisionTimes.z;
      collisionAxis = POSZ;
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
  if (abs(value) > abs(cap)) {
    return value;
  }
  return std::min(std::max(value + change, -abs(cap)), abs(cap));
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
  std::cout << "-------------------------" <<std::endl;

  std::cout << "P: (" << position.x << ", " << position.y << ", " << position.z << ")"<< std::endl;
  // gravity
  accelerate({0, -0.001, 0});
  // player motion
  //accelerate(nextStep);
  float maxSpeed = 0.03;
  if (axisValue(nextStep)) {
    glm::vec3 walkDirection = glm::normalize(nextStep);
    glm::vec3 cap = walkDirection * maxSpeed;
    //glm::vec3 cap = velocityCap - velocity;
    velocity = glm::vec3(
      capTo(velocity.x, nextStep.x, cap.x),
      capTo(velocity.y, nextStep.y, cap.y),
      capTo(velocity.z, nextStep.z, cap.z)
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
    std::cout << "R: <" << reactionForce.x << ", " << reactionForce.y << ", " << reactionForce.z << ">" << std::endl;
    velocity += reactionForce;

    // if there's a reaction force on the negative y axis, we can do some things
    glm::vec3 jumpForce(0);
    if (reactionForce.y > 0) {
      if (jumping) {
        jumpForce += glm::vec3(0, 0.08, 0);
      }
      // std::cout << "acceleratin'" << std::endl;
    }
    // friction
    glm::vec3 frictionForce(0);
    // glm::ivec3 ortho1, ortho2;
    // for (glm::ivec3 axis : {POSX, POSY, POSZ}) {
    //   otherAxes(axis, ortho1, ortho2);
    //   float s = axisValue(glm::vec3(ortho1) * velocity);
    //   float t = axisValue(glm::vec3(ortho2) * velocity);
    //   float theta = atan2(t, s);
    //   float r = axisValue(reactionForce * glm::vec3(axis));
    //   float f = glm::min(r * 0.6f, glm::length(velocity));

    //   frictionForce -= glm::vec3(ortho1) * f * cos(theta);
    //   frictionForce -= glm::vec3(ortho2) * f * sin(theta);
    //   // TODO: maybe implement friction maximum
    // }
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

Hitbox Entity::getHitbox() {
  return hitbox;
}

OBJModel Entity::getModel() {
  return UNIT_CUBE();
}
// };

Player::Player(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity):
Entity(entityName, initialPosition, facing, initialVelocity) {
  hitbox = {{2, 2, 2}};//{{12.0/16, 30.0/32, 12.0/16}};
}

OBJModel Player::getModel() {
  return Entity::getModel();
}

EntityGod::EntityGod(World &world): God(world) {};

void EntityGod::update() {
  for (auto &it : world.entities) {
    Entity &entity = it.second;
    entity.update(world);
  }
}

void EntityGod::createEntity(Entity entity) {
  glm::ivec3 spawnChunk = World::blockToChunkCoordinate(glm::ivec3(entity.position));
  // TODO: add name check
  world.entities[entity.name] = entity;
  if (!world.hasChunk(spawnChunk)) {
    // TODO: make a better response here
    std::cout << "CANNOT SPAWN AN ENTITY WHERE THERE IS NO CHUNK" << std::endl;
    return;
  }
  Chunk &chunk = world.getChunk(spawnChunk);
  chunk.entityNames.insert(entity.name);
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
    Chunk &chunk = world.getChunk(chunkCoordinate);
    chunk.entityNames.erase(name);
  }
  world.entities.erase(name);
}

Entity& EntityGod::getEntity(std::string name) {
  return world.entities[name];
}

/*
** ----- TERRAIN  ----------------
*/

TerrainGod::TerrainGod(World &world): God(world) {}

void TerrainGod::generateSpawn() {
  Chunk chunk = {{{0}}};
  chunk.blocks[5][14][5] = STONE;
  for (int z = 0; z < CHUNK_SIZE; z += 1) {
    for (int x = 0; x < CHUNK_SIZE; x += 1) {
      int y = x / 3 + 9;
      chunk.blocks[z][y][x] = STONE;
    }
  }
  int rad = 4;
  for (int z = -rad; z <= rad; z += 1) {
    for (int x = -rad; x <= rad; x += 1) {
      world.setChunk({x, -1, z}, chunk);
    }
  }
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
          if (!Chunk::inBounds(neighbor) || chunk.getBlock(neighbor) != AIR) {
            continue;
          }
          // add the face represented by a block coordinate and a direction
          faces.push_back({blockCoordinate, direction});
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
  int vertexDirection = face.facing.x | face.facing.y | face.facing.z;
  int zero1 = face.facing.x ? face.facing.y ? 2 : 1 : 0;
  int zero2 = (face.facing.y || face.facing.x) ? 2 : 1;
  // std::cout << "zeroes: " << zero1 << ", " << zero2 << " verdir: " << vertexDirection << std::endl;
  int corner[3] = {face.facing.x, face.facing.y, face.facing.z};
  // iterate through 3 indices
  for (int i = vertexDirection < 0 ? 2 : 0, _i = 0; _i < 3; i += vertexDirection, _i += 1) {
    glm::ivec2 offset1 = SQUARE_OFFSETS[TRI1[i % 3]];
    glm::ivec2 offset2 = SQUARE_OFFSETS[TRI2[i % 3]];
    corner[zero1] = offset1.x;
    corner[zero2] = offset1.y;
    // std::cout << "offset index: " << i % 3 << std::endl;
    vertices[_i] = (glm::vec3(corner[0], corner[1], corner[2]) * 0.5f + origin);
    corner[zero1] = offset2.x;
    corner[zero2] = offset2.y;
    vertices[3 + _i] = (glm::vec3(corner[0], corner[1], corner[2]) * 0.5f + origin);
  }
  builder.addSimpleFace(vertices);
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

// setting a chunk will also erase the cached version
void RenderGod::setChunk(glm::ivec3 chunkCoordinate, Chunk chunk) {
  world.setChunk(chunkCoordinate, chunk);
  realm.erase(chunkCoordinate);
  scene.deleteMesh(Chunk::id(chunkCoordinate));
}

RenderGod::RenderGod(World &world, Scene &openGLScene): God(world), scene(openGLScene) {}

// // get the cache of vertices to render
// std::vector<VBOVertex> RenderWorld::getVertexCache() {
//   return worldVertexCache;
// }
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
        if (glm::distance(glm::vec3(chunkCoordinate), glm::vec3(originChunk)) > radius) {
          // std::cout << "out of chunk render sphere" << std::endl;
          continue;
        }
        // if the chunk is already cached, it must be up to date
        if (realm.find(chunkCoordinate) != realm.end()) {
          // std::cout << "chunk already cached" << std::endl;
          continue;
        }
        // TODO: if a chunk does not exist, we should generate it instead of skipping it
        if (!world.hasChunk(chunkCoordinate)) {
          // std::cout << "chunk does not exist" << std::endl;
          continue;
        }
        chunkCount += 1;
          std::cout << "rendering chunk!------------" << std::endl;
        OBJModel model = scaleOBJ(offsetOBJ(world.getChunk(chunkCoordinate).calculateChunkOBJ(), glm::vec3(chunkCoordinate * CHUNK_SIZE)), BLOCK_SCALE);
        model.vertexNormals.push_back({0, 0, 0});
        scene.createMesh(Chunk::id(chunkCoordinate), model);
      }
    }
  }
  std::cout <<"Rendering THIS MANY CHUNKS: " << chunkCount << std::endl;
}

#endif