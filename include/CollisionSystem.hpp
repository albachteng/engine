#pragma once

#include "SpatialPartition.hpp"
#include "EntityManager.h"
#include <memory>
#include <vector>

/**
 * CollisionSystem - Optimized collision detection using spatial partitioning
 * 
 * Replaces O(N²) collision detection with O(N) or O(N log N) performance
 * using configurable spatial partitioning strategies.
 * 
 * Compatible with existing CAABB and CTransform3D components.
 */
class CollisionSystem {
private:
    std::unique_ptr<SpatialPartitionStrategy> m_spatialPartition;
    glm::vec3 m_worldMin;
    glm::vec3 m_worldMax;
    
public:
    /**
     * Initialize collision system with spatial partitioning strategy
     * @param type Spatial partitioning algorithm to use
     * @param worldMin Minimum world bounds
     * @param worldMax Maximum world bounds  
     * @param cellSize Grid cell size (for uniform grid)
     */
    CollisionSystem(PartitionType type = PartitionType::UNIFORM_GRID,
                   const glm::vec3& worldMin = glm::vec3(-100.0f),
                   const glm::vec3& worldMax = glm::vec3(100.0f),
                   float cellSize = 10.0f);
    
    /**
     * Update collision system with all entities that have CAABB and CTransform3D
     * This should be called once per frame before collision detection
     */
    void updateEntities(EntityManager& entityManager);
    
    /**
     * Find all collision pairs using spatial partitioning
     * @return Vector of entity pairs that are colliding
     */
    std::vector<CollisionPair> findCollisions();
    
    /**
     * Find entities colliding with a specific entity
     * @param entity Target entity to check collisions against
     * @return Vector of entities colliding with target
     */
    std::vector<std::shared_ptr<Entity>> findCollisionsFor(
        const std::shared_ptr<Entity>& entity, EntityManager& entityManager);
    
    /**
     * Find entities within a region
     * @param region World-space AABB to query
     * @param entityManager Entity manager to get entities from
     * @return Vector of entities in the region
     */
    std::vector<std::shared_ptr<Entity>> queryRegion(
        const CAABB& region, EntityManager& entityManager);
    
    /**
     * Find entities near a point
     * @param point World position
     * @param radius Search radius
     * @param entityManager Entity manager to get entities from
     * @return Vector of entities within radius
     */
    std::vector<std::shared_ptr<Entity>> queryRadius(
        const glm::vec3& point, float radius, EntityManager& entityManager);
    
    /**
     * Get performance statistics for the collision system
     */
    PartitionStats getStatistics() const;
    
    /**
     * Clear all entities from the collision system
     * Should be called when changing scenes
     */
    void clear();
    
    /**
     * Check if collision system is properly initialized
     */
    bool isValid() const;
};