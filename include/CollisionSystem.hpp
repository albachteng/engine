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
     * @brief Initialize collision system with spatial partitioning strategy
     * @param type Spatial partitioning algorithm to use (UNIFORM_GRID, QUADTREE, etc.)
     * @param worldMin Minimum world bounds for spatial structure
     * @param worldMax Maximum world bounds for spatial structure
     * @param cellSize Grid cell size (for uniform grid) or leaf size (for trees)
     * 
     * Creates optimized spatial data structure for collision detection.
     * World bounds should encompass all entities that may participate in collisions.
     */
    CollisionSystem(PartitionType type = PartitionType::UNIFORM_GRID,
                   const glm::vec3& worldMin = glm::vec3(-100.0f),
                   const glm::vec3& worldMax = glm::vec3(100.0f),
                   float cellSize = 10.0f);
    
    /**
     * @brief Update collision system with all entities that have CAABB and CTransform3D
     * @param entityManager Entity manager containing entities to check for collisions
     * 
     * Rebuilds spatial partitioning structure with current entity positions and bounds.
     * Must be called once per frame before collision detection to ensure accuracy.
     * Only processes entities that have both CAABB and CTransform3D components.
     */
    void updateEntities(EntityManager& entityManager);
    
    /**
     * @brief Find all collision pairs using spatial partitioning
     * @return Vector of entity pairs that are colliding (AABB intersection)
     * 
     * Uses spatial partitioning to efficiently find potential collision pairs,
     * then performs precise AABB intersection tests. Complexity depends on
     * spatial partitioning strategy: O(N) for uniform grid, O(N log N) for trees.
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
     * @brief Find entities within a region using spatial queries
     * @param region World-space AABB to query
     * @param entityManager Entity manager to get entities from
     * @return Vector of entities whose AABBs intersect with the query region
     * 
     * Efficient spatial query for region-based operations like:
     * - Explosion damage areas
     * - Trigger volumes
     * - Area-of-effect calculations
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