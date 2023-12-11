#include "gravity.hpp"
#include <iostream>

GravitySimulation::GravitySimulation(const float gravitationalConstant) {
  G = gravitationalConstant;
}

bool GravitySimulation::addBody(std::string name, GravitationalBody* body) {
  if (bodies.find(name) != bodies.end()) {
    return false;
  }
  names.push_back(name);
  bodies[name] = body;
  return true;
}

GravitationalBody GravitySimulation::getBody(std::string name) {
  return *bodies[name];
}

void GravitySimulation::impartGravitationalEffects() {
  for (int i = 0; i < names.size(); i += 1) {
    for (int j = i + 1; j < names.size(); j += 1) {
      GravitationalBody *body1 = bodies.at(names.at(i));
      GravitationalBody *body2 = bodies.at(names.at(j));

      float distance = glm::distance(body1->position, body2->position);
      glm::vec3 direction = glm::normalize(body2->position - body1->position);

      float magnitude = G * body1->mass * body2->mass / (distance * distance);
      glm::vec3 gravity = magnitude * direction;

      body1->velocity += gravity / body1->mass;
      body2->velocity -= gravity / body2->mass;
    }
  }
}

void GravitySimulation::updateBodyPositions() {
  for (auto entries : bodies) {
    GravitationalBody *body = entries.second;
    body->position += body->velocity;
  }
}

void GravitySimulation::advance() {
  impartGravitationalEffects();
  updateBodyPositions();
}