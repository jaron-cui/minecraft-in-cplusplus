#include "World.hpp"

class EntityGod: public God {
  public:
    void update() override;
    bool seesEntity(std::string name);
    void createEntity(Entity entity);
    void removeEntity(std::string name);
};