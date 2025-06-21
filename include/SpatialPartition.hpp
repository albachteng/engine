#pragma once
#include "Entity.hpp"
#include "Component.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

/**
 * Spatial Partitioning System for Optimized Collision Detection
 * 
 * Replaces O(N²) collision detection with spatial data structures
 * that achieve O(N) or O(N log N) performance depending on strategy.
 * 
 * Uses Strategy Pattern to allow easy swapping of partitioning algorithms.
 * Compatible with existing CAABB component from Component.h
 */

// Type aliases for clarity
using EntityID = size_t;
using CollisionPair = std::pair<EntityID, EntityID>;

/**
 * Utility functions for working with existing CAABB component
 */
namespace AABBUtils {
    /**
     * Check if two CAABB components intersect
     * Compatible with existing AABBIntersect function but with cleaner interface
     */
    inline bool intersects(const CAABB& a, const CAABB& b) {
        return (a.max.x > b.min.x && a.min.x < b.max.x) &&
               (a.max.y > b.min.y && a.min.y < b.max.y) &&
               (a.max.z > b.min.z && a.min.z < b.max.z);
    }
    
    /**
     * Get world-space AABB for an entity
     * Combines CTransform3D position with CAABB bounds
     */
    CAABB getWorldAABB(const std::shared_ptr<Entity>& entity);
    
    /**
     * Create CAABB from center point and half-extents
     * Helper for spatial partition implementations
     */
    inline CAABB fromCenterAndExtents(const glm::vec3& center, const glm::vec3& halfExtents) {
        return CAABB(center, halfExtents);
    }
    
    /**
     * Get center point of CAABB
     */
    inline glm::vec3 getCenter(const CAABB& aabb) {
        return (aabb.min + aabb.max) * 0.5f;
    }
    
    /**
     * Get size (full extents) of CAABB
     */
    inline glm::vec3 getSize(const CAABB& aabb) {
        return aabb.max - aabb.min;
    }
    
    /**
     * Get half-extents of CAABB
     */
    inline glm::vec3 getHalfExtents(const CAABB& aabb) {
        return (aabb.max - aabb.min) * 0.5f;
    }
    
    /**
     * Check if CAABB is valid (min <= max)
     */
    inline bool isValid(const CAABB& aabb) {
        return aabb.min.x <= aabb.max.x && 
               aabb.min.y <= aabb.max.y && 
               aabb.min.z <= aabb.max.z;
    }
    
    /**
     * Expand CAABB to include a point
     */
    inline CAABB expandToInclude(const CAABB& aabb, const glm::vec3& point) {
        CAABB result;
        result.min = glm::min(aabb.min, point);
        result.max = glm::max(aabb.max, point);
        return result;
    }
    
    /**
     * Expand CAABB to include another AABB
     */
    inline CAABB expandToInclude(const CAABB& a, const CAABB& b) {
        CAABB result;
        result.min = glm::min(a.min, b.min);
        result.max = glm::max(a.max, b.max);
        return result;
    }
}

/**
 * Performance statistics for spatial partitioning algorithms
 * Used for debugging and performance comparison
 */
struct PartitionStats {
    size_t totalNodes = 0;           // Total number of nodes in structure
    size_t maxDepth = 0;             // Maximum depth of structure
    size_t totalEntities = 0;        // Total entities being tracked
    size_t emptyNodes = 0;           // Nodes with no entities
    size_t averageEntitiesPerNode = 0; // Average entities per non-empty node
    size_t maxEntitiesInSingleNode = 0; // Max entities in any single node
    
    // Performance metrics
    double lastUpdateTimeMs = 0.0;   // Time for last update operation
    double lastQueryTimeMs = 0.0;    // Time for last collision query
    size_t totalCollisionChecks = 0; // Number of AABB tests performed
    
    void reset() {
        totalNodes = maxDepth = totalEntities = emptyNodes = 0;
        averageEntitiesPerNode = maxEntitiesInSingleNode = 0;
        lastUpdateTimeMs = lastQueryTimeMs = 0.0;
        totalCollisionChecks = 0;
    }
};

/**
 * Abstract base class for spatial partitioning strategies
 * 
 * This interface allows easy swapping of different spatial partitioning
 * algorithms (uniform grid, quadtree, octree, spatial hashing, etc.)
 * 
 * Uses existing CAABB component for compatibility with current codebase
 */
class SpatialPartitionStrategy {
public:
    virtual ~SpatialPartitionStrategy() = default;
    
    // ========================================================================
    // Core Entity Management Interface
    // ========================================================================
    
    /**
     * Insert an entity into the spatial structure
     * @param entityId Unique identifier for the entity
     * @param bounds World-space CAABB of the entity
     */
    virtual void insert(EntityID entityId, const CAABB& bounds) = 0;
    
    /**
     * Remove an entity from the spatial structure
     * @param entityId Entity to remove
     */
    virtual void remove(EntityID entityId) = 0;
    
    /**
     * Update an entity's position in the spatial structure
     * More efficient than remove + insert for moving entities
     * @param entityId Entity to update
     * @param newBounds New world-space CAABB
     */
    virtual void update(EntityID entityId, const CAABB& newBounds) = 0;
    
    /**
     * Clear all entities from the spatial structure
     * Used when switching scenes or resetting
     */
    virtual void clear() = 0;
    
    // ========================================================================
    // Query Interface
    // ========================================================================
    
    /**
     * Find all entities within a specific region
     * @param region World-space CAABB to query
     * @return Vector of entity IDs that intersect the region
     */
    virtual std::vector<EntityID> query(const CAABB& region) const = 0;
    
    /**
     * Find all potential collision pairs
     * This is the main function for collision detection optimization
     * @return Vector of entity ID pairs that might be colliding
     */
    virtual std::vector<CollisionPair> findCollisions() const = 0;
    
    /**
     * Find all entities near a specific point
     * @param point World-space point
     * @param radius Search radius
     * @return Vector of entity IDs within radius of point
     */
    virtual std::vector<EntityID> queryRadius(const glm::vec3& point, float radius) const = 0;
    
    // ========================================================================
    // Performance and Debugging Interface
    // ========================================================================
    
    /**
     * Get performance statistics for this partitioning strategy
     * @param stats Output structure to fill with statistics
     */
    virtual void getStatistics(PartitionStats& stats) const = 0;
    
    /**
     * Get a human-readable name for this strategy
     * Used for debugging and performance comparison
     */
    virtual const char* getStrategyName() const = 0;
    
    /**
     * Check if the spatial structure is in a valid state
     * Used for debugging and unit tests
     */
    virtual bool isValid() const = 0;
};

/**
 * Factory function for creating spatial partition strategies
 * Makes it easy to switch between different implementations
 */
enum class PartitionType {
    UNIFORM_GRID,    // Simple grid-based partitioning - IMPLEMENTED
    QUADTREE,        // Adaptive 2D tree - TODO: Future implementation
    OCTREE,          // Adaptive 3D tree - TODO: Future implementation  
    SPATIAL_HASH     // Hash-based infinite partitioning - TODO: Future implementation
};

/**
 * Create a spatial partitioning strategy
 * @param type Type of partitioning to create
 * @param worldMin Minimum bounds of the world
 * @param worldMax Maximum bounds of the world
 * @param cellSize Grid cell size (for uniform grid)
 * @return Unique pointer to the created strategy
 */
std::unique_ptr<SpatialPartitionStrategy> createSpatialPartition(
    PartitionType type,
    const glm::vec3& worldMin = glm::vec3(-100.0f),
    const glm::vec3& worldMax = glm::vec3(100.0f),
    float cellSize = 10.0f
);