#include "../include/MovementSystem.hpp"
#include "../include/Logger.hpp"
#include "../include/Constants.hpp"
#include <glm/gtc/matrix_transform.hpp>

void MovementSystem::updateMovement(EntityManager& entityManager, float deltaTime) {
    int entitiesUpdated = 0;
    
    for (auto& entity : entityManager.getEntities()) {
        if (!entity->has<CTransform3D>() || !entity->has<CMovement3D>()) {
            continue;
        }
        
        // Apply accumulated forces
        auto forceIt = m_accumulatedForces.find(entity->id());
        if (forceIt != m_accumulatedForces.end()) {
            addForce(entity, forceIt->second);
            m_accumulatedForces.erase(forceIt);
        }
        
        // Update velocity from acceleration
        updateVelocity(entity, deltaTime);
        
        // Apply speed limits
        applySpeedLimit(entity);
        
        // Update position from velocity
        updatePosition(entity, deltaTime);
        
        // Apply rotation updates
        applyRotation(entity, deltaTime);
        
        entitiesUpdated++;
    }
    
    if (entitiesUpdated > 0) {
        LOG_DEBUG_STREAM("MovementSystem: Updated " << entitiesUpdated << " entities");
    }
}

void MovementSystem::applyImpulse(std::shared_ptr<Entity> entity, const glm::vec3& impulse) {
    if (!entity->has<CMovement3D>()) {
        return;
    }
    
    auto& movement = entity->get<CMovement3D>();
    // Impulse directly changes velocity (assuming unit mass)
    movement.vel += impulse;
    
    LOG_DEBUG_STREAM("MovementSystem: Applied impulse (" << impulse.x << ", " << impulse.y << ", " << impulse.z 
                    << ") to entity " << entity->id());
}

void MovementSystem::setVelocity(std::shared_ptr<Entity> entity, const glm::vec3& velocity) {
    if (!entity->has<CMovement3D>()) {
        return;
    }
    
    auto& movement = entity->get<CMovement3D>();
    movement.vel = velocity;
}

void MovementSystem::addForce(std::shared_ptr<Entity> entity, const glm::vec3& force) {
    if (!entity->has<CMovement3D>()) {
        return;
    }
    
    // Accumulate forces to be applied in next update
    m_accumulatedForces[entity->id()] += force;
}

void MovementSystem::setMaxSpeed(std::shared_ptr<Entity> entity, float maxSpeed) {
    m_entityMaxSpeeds[entity->id()] = maxSpeed;
}

void MovementSystem::applyDamping(std::shared_ptr<Entity> entity, float dampingFactor) {
    if (!entity->has<CMovement3D>()) {
        return;
    }
    
    auto& movement = entity->get<CMovement3D>();
    movement.vel *= dampingFactor;
    movement.acc *= dampingFactor;
}

void MovementSystem::updatePosition(std::shared_ptr<Entity> entity, float deltaTime) {
    auto& transform = entity->get<CTransform3D>();
    const auto& movement = entity->get<CMovement3D>();
    
    // Basic Euler integration: position += velocity * deltaTime
    transform.position += movement.vel * deltaTime;
}

void MovementSystem::updateVelocity(std::shared_ptr<Entity> entity, float deltaTime) {
    auto& movement = entity->get<CMovement3D>();
    
    // Basic Euler integration: velocity += acceleration * deltaTime
    movement.vel += movement.acc * deltaTime;
}

void MovementSystem::applySpeedLimit(std::shared_ptr<Entity> entity) {
    auto speedIt = m_entityMaxSpeeds.find(entity->id());
    if (speedIt == m_entityMaxSpeeds.end()) {
        return; // No speed limit set for this entity
    }
    
    auto& movement = entity->get<CMovement3D>();
    float currentSpeed = glm::length(movement.vel);
    
    if (currentSpeed > speedIt->second) {
        // Normalize velocity and scale to max speed
        movement.vel = glm::normalize(movement.vel) * speedIt->second;
    }
}

void MovementSystem::applyRotation(std::shared_ptr<Entity> entity, float deltaTime) {
    auto& transform = entity->get<CTransform3D>();
    
    // Apply constant rotation (from existing GameScene logic)
    glm::vec3 rotationDelta = glm::vec3(EngineConstants::World::ENTITY_ROTATION_RATE,
                                       EngineConstants::World::ENTITY_ROTATION_RATE,
                                       EngineConstants::World::ENTITY_ROTATION_RATE) * deltaTime;
    transform.rotation += rotationDelta;
    
}