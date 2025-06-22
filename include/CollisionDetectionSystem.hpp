#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct CollisionEvent {
    std::shared_ptr<Entity> entityA;
    std::shared_ptr<Entity> entityB;
    glm::vec3 contactPoint;
    glm::vec3 contactNormal;
    float penetrationDepth;
    
    CollisionEvent(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
        : entityA(a), entityB(b), contactPoint(0.0f), contactNormal(0.0f), penetrationDepth(0.0f) {}
};

class CollisionDetectionSystem {
public:
    CollisionDetectionSystem() = default;
    ~CollisionDetectionSystem() = default;

    std::vector<CollisionEvent> detectCollisions(EntityManager& entityManager);
    
    bool checkAABBCollision(const CAABB& a, const CAABB& b);
    
    CollisionEvent calculateCollisionDetails(std::shared_ptr<Entity> entityA, std::shared_ptr<Entity> entityB);
    
    void updateAABBForEntity(std::shared_ptr<Entity> entity);

private:
    CAABB getWorldAABB(std::shared_ptr<Entity> entity);
    
    glm::vec3 calculateContactPoint(const CAABB& a, const CAABB& b);
    glm::vec3 calculateContactNormal(const CAABB& a, const CAABB& b);
    float calculatePenetrationDepth(const CAABB& a, const CAABB& b);
};