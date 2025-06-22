#include "../include/CollisionDetectionSystem.hpp"
#include "../include/Logger.hpp"
#include <algorithm>

std::vector<CollisionEvent> CollisionDetectionSystem::detectCollisions(EntityManager& entityManager) {
    std::vector<CollisionEvent> collisions;
    auto entities = entityManager.getEntities();
    
    // Update AABB for all entities first
    for (auto& entity : entities) {
        if (entity->has<CAABB>() && entity->has<CTransform3D>()) {
            updateAABBForEntity(entity);
        }
    }
    
    // O(N²) collision detection - can be optimized with spatial partitioning later
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            auto entityA = entities[i];
            auto entityB = entities[j];
            
            // Only check entities with collision components
            if (!entityA->has<CAABB>() || !entityB->has<CAABB>()) {
                continue;
            }
            
            // Skip if either entity doesn't have transform
            if (!entityA->has<CTransform3D>() || !entityB->has<CTransform3D>()) {
                continue;
            }
            
            CAABB aabbA = entityA->get<CAABB>();
            CAABB aabbB = entityB->get<CAABB>();
            
            if (checkAABBCollision(aabbA, aabbB)) {
                CollisionEvent collision = calculateCollisionDetails(entityA, entityB);
                collisions.push_back(collision);
                
                LOG_DEBUG_STREAM("CollisionDetectionSystem: Collision detected between entities " 
                                << entityA->id() << " and " << entityB->id());
            }
        }
    }
    
    LOG_DEBUG_STREAM("CollisionDetectionSystem: Detected " << collisions.size() << " collisions");
    return collisions;
}

bool CollisionDetectionSystem::checkAABBCollision(const CAABB& a, const CAABB& b) {
    return (a.max.x > b.min.x && a.min.x < b.max.x) &&
           (a.max.y > b.min.y && a.min.y < b.max.y) &&
           (a.max.z > b.min.z && a.min.z < b.max.z);
}

CollisionEvent CollisionDetectionSystem::calculateCollisionDetails(std::shared_ptr<Entity> entityA, std::shared_ptr<Entity> entityB) {
    CollisionEvent collision(entityA, entityB);
    
    const CAABB& aabbA = entityA->get<CAABB>();
    const CAABB& aabbB = entityB->get<CAABB>();
    
    collision.contactPoint = calculateContactPoint(aabbA, aabbB);
    collision.contactNormal = calculateContactNormal(aabbA, aabbB);
    collision.penetrationDepth = calculatePenetrationDepth(aabbA, aabbB);
    
    return collision;
}

void CollisionDetectionSystem::updateAABBForEntity(std::shared_ptr<Entity> entity) {
    if (!entity->has<CTransform3D>() || !entity->has<CAABB>()) {
        return;
    }
    
    // For now, we'll assume CAABB components are updated externally
    // In a more complete system, we'd calculate AABB from CTriangle vertices here
    const auto& transform = entity->get<CTransform3D>();
    auto& aabb = entity->get<CAABB>();
    
    // Simple approach: translate the AABB to the entity's position
    // This assumes the CAABB was created relative to origin
    glm::vec3 center = (aabb.min + aabb.max) * 0.5f;
    glm::vec3 extents = (aabb.max - aabb.min) * 0.5f;
    
    aabb.min = transform.position - extents;
    aabb.max = transform.position + extents;
}

CAABB CollisionDetectionSystem::getWorldAABB(std::shared_ptr<Entity> entity) {
    if (!entity->has<CTransform3D>() || !entity->has<CAABB>()) {
        return CAABB(); // Return default AABB
    }
    
    const auto& transform = entity->get<CTransform3D>();
    const auto& localAABB = entity->get<CAABB>();
    
    CAABB worldAABB;
    worldAABB.min = localAABB.min + transform.position;
    worldAABB.max = localAABB.max + transform.position;
    
    return worldAABB;
}

glm::vec3 CollisionDetectionSystem::calculateContactPoint(const CAABB& a, const CAABB& b) {
    // Calculate the contact point as the center of the overlapping region
    glm::vec3 contactMin = glm::max(a.min, b.min);
    glm::vec3 contactMax = glm::min(a.max, b.max);
    
    return (contactMin + contactMax) * 0.5f;
}

glm::vec3 CollisionDetectionSystem::calculateContactNormal(const CAABB& a, const CAABB& b) {
    // Calculate the separation on each axis
    glm::vec3 centerA = (a.min + a.max) * 0.5f;
    glm::vec3 centerB = (b.min + b.max) * 0.5f;
    glm::vec3 separation = centerB - centerA;
    
    // Find the axis with the smallest overlap (minimum separation)
    glm::vec3 overlap;
    overlap.x = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
    overlap.y = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
    overlap.z = std::min(a.max.z - b.min.z, b.max.z - a.min.z);
    
    // The normal points from A to B along the axis of minimum overlap
    if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
        return glm::vec3(separation.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
    } else if (overlap.y <= overlap.z) {
        return glm::vec3(0.0f, separation.y > 0 ? 1.0f : -1.0f, 0.0f);
    } else {
        return glm::vec3(0.0f, 0.0f, separation.z > 0 ? 1.0f : -1.0f);
    }
}

float CollisionDetectionSystem::calculatePenetrationDepth(const CAABB& a, const CAABB& b) {
    // Calculate the overlap on each axis
    float overlapX = std::min(a.max.x - b.min.x, b.max.x - a.min.x);
    float overlapY = std::min(a.max.y - b.min.y, b.max.y - a.min.y);
    float overlapZ = std::min(a.max.z - b.min.z, b.max.z - a.min.z);
    
    // Return the minimum overlap (deepest penetration)
    return std::min({overlapX, overlapY, overlapZ});
}