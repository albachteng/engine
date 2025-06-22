#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <glm/glm.hpp>
#include <memory>

enum class BoundaryAction {
    BOUNCE,     // Reflect velocity and clamp position
    WRAP,       // Teleport to opposite side
    CLAMP,      // Stop at boundary
    DESTROY     // Remove entity when it hits boundary
};

struct BoundaryConstraint {
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    BoundaryAction action;
    float damping;      // Energy loss factor for bouncing (0.0 = no bounce, 1.0 = perfect bounce)
    
    BoundaryConstraint() 
        : minBounds(0.0f), maxBounds(0.0f), action(BoundaryAction::BOUNCE), damping(0.9f) {}
    
    BoundaryConstraint(const glm::vec3& minB, const glm::vec3& maxB, 
                      BoundaryAction act = BoundaryAction::BOUNCE, float damp = 0.9f)
        : minBounds(minB), maxBounds(maxB), action(act), damping(damp) {}
};

class BoundarySystem {
public:
    BoundarySystem() = default;
    BoundarySystem(const BoundaryConstraint& constraint);
    ~BoundarySystem() = default;

    void enforceBoundaries(EntityManager& entityManager);
    
    void setBoundaryConstraint(const BoundaryConstraint& constraint);
    
    void setEntityBoundaryAction(EntityTag tag, BoundaryAction action, float damping = 0.9f);
    
    bool isEntityOutOfBounds(std::shared_ptr<Entity> entity) const;
    
    glm::vec3 getViolatedBoundaries(std::shared_ptr<Entity> entity) const;

private:
    void handleBoundaryViolation(std::shared_ptr<Entity> entity, const glm::vec3& violations);
    
    void applyBounceAction(std::shared_ptr<Entity> entity, const glm::vec3& violations, float damping);
    
    void applyWrapAction(std::shared_ptr<Entity> entity, const glm::vec3& violations);
    
    void applyClampAction(std::shared_ptr<Entity> entity, const glm::vec3& violations);
    
    BoundaryAction getBoundaryActionForEntity(std::shared_ptr<Entity> entity) const;
    
    float getDampingForEntity(std::shared_ptr<Entity> entity) const;
    
    BoundaryConstraint m_globalConstraint;
    std::unordered_map<EntityTag, BoundaryAction> m_entityActions;
    std::unordered_map<EntityTag, float> m_entityDamping;
    std::vector<std::shared_ptr<Entity>> m_entitiesToDestroy;
};