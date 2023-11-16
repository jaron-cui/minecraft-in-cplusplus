#include "World.hpp"

class EntityGod: public God {
  public:
    EntityGod(World &world);
    void update() override;
    bool seesEntity(std::string name);
    void createEntity(Entity entity);
    void removeEntity(std::string name);
    Entity getEntity(std::string name);
};