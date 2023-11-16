#include "world.hpp"

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
    // progress this god's actions
    void update();
};

class Entity {
  protected:
    std::string name;
    glm::vec3 position;
    glm::vec3 velocity;
    float theta;
    int health;
  public:
    Entity(std::string entityName, glm::vec3 initialPosition, float facing, glm::vec3 initialVelocity);
    // update the entity's position, behavior, etc...
    void update(World &world);
    std::string getName() const;
    void setPosition(glm::vec3 to);
    void setVelocity(glm::vec3 to);
    void accelerate(glm::vec3 acceleration);
    virtual OBJModel getModel();
    friend class EntityGod;
};

class Player: public Entity {
  OBJModel getModel() override;
};

class EntityGod: public God {
  public:
    bool seesEntity(std::string name);
    void createEntity(Entity entity);
    void removeEntity(std::string name);
};