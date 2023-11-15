#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <unordered_map>

// represents a gravitational body with Newtonian properties
class GravitationalBody {
  public:
  glm::vec3 position;
  glm::vec3 velocity;
  float mass;
};

// represents a self-contained simulation of multiple gravitational bodies
class GravitySimulation {
  private:
    // the gravitational constant
    float G;
    // the names of gravitational bodies in the simulation
    std::vector<std::string> names;
    // a mapping from names to bodies
    std::unordered_map<std::string, GravitationalBody*> bodies;
  public:
    // constructor
    GravitySimulation(const float gravitationalConstant);
    // add a gravitational body to the simulation
    bool addBody(std::string name, GravitationalBody* body);
    // retrieve a gravitational body by name
    GravitationalBody getBody(std::string name);
    // exert gravitational forces on each body
    void impartGravitationalEffects();
    // step the positions of bodies by their velocities
    void updateBodyPositions();
    // advance the simulation by applying forces and updating positions
    void advance();
};

//float G = 0.0000001;
