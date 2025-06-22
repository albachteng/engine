#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <memory>

class MovementSystem {
public:
    MovementSystem() = default;
    ~MovementSystem() = default;

    void updateMovement(EntityManager& entityManager, float deltaTime);
    
    void applyImpulse(std::shared_ptr<Entity> entity, const glm::vec3& impulse);
    
    void setVelocity(std::shared_ptr<Entity> entity, const glm::vec3& velocity);
    
    void addForce(std::shared_ptr<Entity> entity, const glm::vec3& force);
    
    void setMaxSpeed(std::shared_ptr<Entity> entity, float maxSpeed);
    
    void applyDamping(std::shared_ptr<Entity> entity, float dampingFactor);

private:
    void updatePosition(std::shared_ptr<Entity> entity, float deltaTime);
    
    void updateVelocity(std::shared_ptr<Entity> entity, float deltaTime);
    
    void applySpeedLimit(std::shared_ptr<Entity> entity);
    
    void applyRotation(std::shared_ptr<Entity> entity, float deltaTime);
    
    std::unordered_map<size_t, float> m_entityMaxSpeeds;
    std::unordered_map<size_t, glm::vec3> m_accumulatedForces;
};