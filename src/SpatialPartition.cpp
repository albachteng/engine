#include "../include/SpatialPartition.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_set>

// ============================================================================
// AABBUtils Implementation
// ============================================================================

CAABB AABBUtils::getWorldAABB(const std::shared_ptr<Entity>& entity) {
    if (!entity->has<CTransform3D>() || !entity->has<CAABB>()) {
        // Return invalid AABB for entities without required components
        CAABB invalid;
        invalid.min = glm::vec3(0.0f);
        invalid.max = glm::vec3(0.0f);
        return invalid;
    }
    
    const auto& transform = entity->get<CTransform3D>();
    const auto& localAABB = entity->get<CAABB>();
    
    // Transform local AABB to world space by adding entity position
    // Note: This assumes the CAABB is relative to entity center
    // For more complex transformations (rotation/scale), this would need to be expanded
    CAABB worldAABB;
    worldAABB.min = localAABB.min + transform.position;
    worldAABB.max = localAABB.max + transform.position;
    
    return worldAABB;
}

// ============================================================================
// UniformGrid Implementation
// ============================================================================

/**
 * Uniform Grid Spatial Partitioning Strategy
 * 
 * Divides world space into a regular grid of cells.
 * Each entity is placed in cells it overlaps.
 * 
 * Pros:
 * - Simple and fast O(1) insertion/lookup
 * - Great for evenly distributed entities
 * - Predictable memory usage
 * 
 * Cons:
 * - Poor for clustered entities (many empty cells)
 * - Fixed world bounds
 * - Large entities span many cells
 */
class UniformGridStrategy : public SpatialPartitionStrategy {
private:
    struct GridCell {
        std::vector<EntityID> entities;
        
        void addEntity(EntityID id) {
            entities.push_back(id);
        }
        
        void removeEntity(EntityID id) {
            auto it = std::find(entities.begin(), entities.end(), id);
            if (it != entities.end()) {
                entities.erase(it);
            }
        }
        
        bool isEmpty() const {
            return entities.empty();
        }
    };
    
    // Grid configuration
    glm::vec3 m_worldMin;
    glm::vec3 m_worldMax;
    glm::vec3 m_worldSize;
    float m_cellSize;
    glm::ivec3 m_gridDimensions;
    
    // Grid storage
    std::vector<GridCell> m_grid;
    std::unordered_map<EntityID, CAABB> m_entityBounds; // Track entity positions for updates
    
    // Performance tracking
    mutable PartitionStats m_stats;
    
public:
    UniformGridStrategy(const glm::vec3& worldMin, const glm::vec3& worldMax, float cellSize)
        : m_worldMin(worldMin), m_worldMax(worldMax), m_cellSize(cellSize) {
        
        m_worldSize = m_worldMax - m_worldMin;
        m_gridDimensions = glm::ivec3(
            static_cast<int>(std::ceil(m_worldSize.x / m_cellSize)),
            static_cast<int>(std::ceil(m_worldSize.y / m_cellSize)),
            static_cast<int>(std::ceil(m_worldSize.z / m_cellSize))
        );
        
        // Allocate grid cells
        size_t totalCells = m_gridDimensions.x * m_gridDimensions.y * m_gridDimensions.z;
        m_grid.resize(totalCells);
        
        std::cout << "UniformGrid initialized: " << m_gridDimensions.x << "x" 
                  << m_gridDimensions.y << "x" << m_gridDimensions.z 
                  << " = " << totalCells << " cells" << std::endl;
    }
    
    // Convert world position to grid coordinates
    glm::ivec3 worldToGrid(const glm::vec3& worldPos) const {
        glm::vec3 relative = worldPos - m_worldMin;
        return glm::ivec3(
            static_cast<int>(relative.x / m_cellSize),
            static_cast<int>(relative.y / m_cellSize),
            static_cast<int>(relative.z / m_cellSize)
        );
    }
    
    // Convert grid coordinates to grid index
    size_t gridToIndex(const glm::ivec3& gridPos) const {
        // Clamp to valid range
        glm::ivec3 clamped = glm::clamp(gridPos, glm::ivec3(0), m_gridDimensions - 1);
        return clamped.z * m_gridDimensions.x * m_gridDimensions.y + 
               clamped.y * m_gridDimensions.x + 
               clamped.x;
    }
    
    // Check if grid coordinates are valid
    bool isValidGridPos(const glm::ivec3& gridPos) const {
        return gridPos.x >= 0 && gridPos.x < m_gridDimensions.x &&
               gridPos.y >= 0 && gridPos.y < m_gridDimensions.y &&
               gridPos.z >= 0 && gridPos.z < m_gridDimensions.z;
    }
    
    // Get all grid cells that an AABB overlaps
    std::vector<size_t> getOverlappingCells(const CAABB& bounds) const {
        std::vector<size_t> cells;
        
        glm::ivec3 minGrid = worldToGrid(bounds.min);
        glm::ivec3 maxGrid = worldToGrid(bounds.max);
        
        for (int z = minGrid.z; z <= maxGrid.z; ++z) {
            for (int y = minGrid.y; y <= maxGrid.y; ++y) {
                for (int x = minGrid.x; x <= maxGrid.x; ++x) {
                    glm::ivec3 gridPos(x, y, z);
                    if (isValidGridPos(gridPos)) {
                        cells.push_back(gridToIndex(gridPos));
                    }
                }
            }
        }
        
        return cells;
    }
    
    void insert(EntityID entityId, const CAABB& bounds) override {
        // Store entity bounds for future updates
        m_entityBounds[entityId] = bounds;
        
        // Insert into all overlapping cells
        auto cells = getOverlappingCells(bounds);
        for (size_t cellIndex : cells) {
            m_grid[cellIndex].addEntity(entityId);
        }
    }
    
    void remove(EntityID entityId) override {
        auto it = m_entityBounds.find(entityId);
        if (it == m_entityBounds.end()) {
            return; // Entity not found
        }
        
        // Remove from all cells it was in
        auto cells = getOverlappingCells(it->second);
        for (size_t cellIndex : cells) {
            m_grid[cellIndex].removeEntity(entityId);
        }
        
        m_entityBounds.erase(it);
    }
    
    void update(EntityID entityId, const CAABB& newBounds) override {
        auto it = m_entityBounds.find(entityId);
        if (it == m_entityBounds.end()) {
            // Entity not found, just insert
            insert(entityId, newBounds);
            return;
        }
        
        const CAABB& oldBounds = it->second;
        
        // Check if entity moved to different cells
        auto oldCells = getOverlappingCells(oldBounds);
        auto newCells = getOverlappingCells(newBounds);
        
        // Remove from old cells that are no longer overlapped
        for (size_t oldCell : oldCells) {
            if (std::find(newCells.begin(), newCells.end(), oldCell) == newCells.end()) {
                m_grid[oldCell].removeEntity(entityId);
            }
        }
        
        // Add to new cells that weren't previously overlapped
        for (size_t newCell : newCells) {
            if (std::find(oldCells.begin(), oldCells.end(), newCell) == oldCells.end()) {
                m_grid[newCell].addEntity(entityId);
            }
        }
        
        // Update stored bounds
        it->second = newBounds;
    }
    
    void clear() override {
        for (auto& cell : m_grid) {
            cell.entities.clear();
        }
        m_entityBounds.clear();
    }
    
    std::vector<EntityID> query(const CAABB& region) const override {
        std::unordered_set<EntityID> result;
        
        auto cells = getOverlappingCells(region);
        for (size_t cellIndex : cells) {
            const auto& cell = m_grid[cellIndex];
            for (EntityID entityId : cell.entities) {
                result.insert(entityId);
            }
        }
        
        return std::vector<EntityID>(result.begin(), result.end());
    }
    
    std::vector<CollisionPair> findCollisions() const override {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<CollisionPair> collisions;
        
        // Custom hash function for std::pair<EntityID, EntityID>
        struct PairHash {
            std::size_t operator()(const std::pair<EntityID, EntityID>& p) const {
                return std::hash<EntityID>{}(p.first) ^ (std::hash<EntityID>{}(p.second) << 1);
            }
        };
        
        std::unordered_set<std::pair<EntityID, EntityID>, PairHash> processedPairs;
        
        size_t totalChecks = 0;
        
        // Check each non-empty cell
        for (const auto& cell : m_grid) {
            if (cell.entities.size() < 2) continue;
            
            // Check all pairs within this cell
            for (size_t i = 0; i < cell.entities.size(); ++i) {
                for (size_t j = i + 1; j < cell.entities.size(); ++j) {
                    EntityID entityA = cell.entities[i];
                    EntityID entityB = cell.entities[j];
                    
                    // Create consistent pair ordering
                    auto pair = std::make_pair(std::min(entityA, entityB), 
                                             std::max(entityA, entityB));
                    
                    // Skip if we've already checked this pair
                    if (processedPairs.find(pair) != processedPairs.end()) {
                        continue;
                    }
                    processedPairs.insert(pair);
                    
                    // Get entity bounds and check for actual collision
                    auto boundsA = m_entityBounds.find(entityA);
                    auto boundsB = m_entityBounds.find(entityB);
                    
                    if (boundsA != m_entityBounds.end() && boundsB != m_entityBounds.end()) {
                        totalChecks++;
                        if (AABBUtils::intersects(boundsA->second, boundsB->second)) {
                            collisions.emplace_back(entityA, entityB);
                        }
                    }
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        m_stats.lastQueryTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
        m_stats.totalCollisionChecks = totalChecks;
        
        return collisions;
    }
    
    std::vector<EntityID> queryRadius(const glm::vec3& point, float radius) const override {
        // Create AABB around the point with radius
        glm::vec3 radiusVec(radius);
        CAABB queryRegion;
        queryRegion.min = point - radiusVec;
        queryRegion.max = point + radiusVec;
        
        // Get potential entities from grid
        auto candidates = query(queryRegion);
        
        // Filter by actual distance
        std::vector<EntityID> result;
        float radiusSquared = radius * radius;
        
        for (EntityID entityId : candidates) {
            auto it = m_entityBounds.find(entityId);
            if (it != m_entityBounds.end()) {
                glm::vec3 entityCenter = AABBUtils::getCenter(it->second);
                glm::vec3 diff = entityCenter - point;
                float distanceSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
                if (distanceSquared <= radiusSquared) {
                    result.push_back(entityId);
                }
            }
        }
        
        return result;
    }
    
    void getStatistics(PartitionStats& stats) const override {
        stats = m_stats;
        stats.totalNodes = m_grid.size();
        stats.maxDepth = 1; // Grid has no depth
        stats.totalEntities = m_entityBounds.size();
        
        // Calculate empty nodes and entity distribution
        size_t emptyNodes = 0;
        size_t totalEntitiesInNodes = 0;
        size_t maxEntitiesInNode = 0;
        
        for (const auto& cell : m_grid) {
            if (cell.isEmpty()) {
                emptyNodes++;
            } else {
                totalEntitiesInNodes += cell.entities.size();
                maxEntitiesInNode = std::max(maxEntitiesInNode, cell.entities.size());
            }
        }
        
        stats.emptyNodes = emptyNodes;
        stats.maxEntitiesInSingleNode = maxEntitiesInNode;
        stats.averageEntitiesPerNode = (stats.totalNodes - emptyNodes) > 0 ? 
            totalEntitiesInNodes / (stats.totalNodes - emptyNodes) : 0;
    }
    
    const char* getStrategyName() const override {
        return "UniformGrid";
    }
    
    bool isValid() const override {
        // Check grid dimensions
        if (m_gridDimensions.x <= 0 || m_gridDimensions.y <= 0 || m_gridDimensions.z <= 0) {
            return false;
        }
        
        // Check that all entities in bounds map exist in grid
        for (const auto& pair : m_entityBounds) {
            EntityID entityId = pair.first;
            const CAABB& bounds = pair.second;
            
            auto cells = getOverlappingCells(bounds);
            bool foundInGrid = false;
            
            for (size_t cellIndex : cells) {
                const auto& cell = m_grid[cellIndex];
                if (std::find(cell.entities.begin(), cell.entities.end(), entityId) != cell.entities.end()) {
                    foundInGrid = true;
                    break;
                }
            }
            
            if (!foundInGrid) {
                return false;
            }
        }
        
        return true;
    }
};

// ============================================================================
// Future Algorithm Stubs
// ============================================================================

/**
 * TODO: Quadtree Implementation
 * 
 * Adaptive 2D spatial partitioning that subdivides space as needed.
 * Excellent for 2D games with non-uniform entity distribution.
 * 
 * Implementation notes:
 * - Each node has 4 children (NW, NE, SW, SE)
 * - Split when node exceeds capacity (e.g., 8 entities)
 * - Merge when children collectively have few entities
 * - Handle entities that span multiple nodes
 */
class QuadtreeStrategy : public SpatialPartitionStrategy {
public:
    QuadtreeStrategy(const glm::vec3& worldMin, const glm::vec3& worldMax, size_t maxEntitiesPerNode = 8) {
        // TODO: Implement quadtree construction
        std::cout << "TODO: QuadtreeStrategy not yet implemented" << std::endl;
    }
    
    void insert(EntityID entityId, const CAABB& bounds) override {
        // TODO: Implement recursive insertion
    }
    
    void remove(EntityID entityId) override {
        // TODO: Implement recursive removal with node merging
    }
    
    void update(EntityID entityId, const CAABB& newBounds) override {
        // TODO: Implement efficient update (may need remove + insert)
    }
    
    void clear() override {
        // TODO: Clear all nodes
    }
    
    std::vector<EntityID> query(const CAABB& region) const override {
        // TODO: Implement recursive region query
        return {};
    }
    
    std::vector<CollisionPair> findCollisions() const override {
        // TODO: Implement collision detection using tree traversal
        return {};
    }
    
    std::vector<EntityID> queryRadius(const glm::vec3& point, float radius) const override {
        // TODO: Implement radius query
        return {};
    }
    
    void getStatistics(PartitionStats& stats) const override {
        // TODO: Calculate tree statistics (depth, nodes, etc.)
        stats.reset();
    }
    
    const char* getStrategyName() const override {
        return "Quadtree (TODO)";
    }
    
    bool isValid() const override {
        // TODO: Validate tree structure
        return true;
    }
};

/**
 * TODO: Spatial Hashing Implementation
 * 
 * Hash-based spatial partitioning for infinite or very large worlds.
 * Similar to uniform grid but uses hash table instead of array.
 * 
 * Implementation notes:
 * - Hash world coordinates to bucket indices
 * - Handle hash collisions gracefully
 * - No fixed world bounds - scales infinitely
 * - Great for streaming/procedural worlds
 */
class SpatialHashStrategy : public SpatialPartitionStrategy {
public:
    SpatialHashStrategy(float cellSize = 10.0f) {
        // TODO: Implement spatial hash construction
        std::cout << "TODO: SpatialHashStrategy not yet implemented" << std::endl;
    }
    
    void insert(EntityID entityId, const CAABB& bounds) override {
        // TODO: Hash entity position and insert into buckets
    }
    
    void remove(EntityID entityId) override {
        // TODO: Remove from hash buckets
    }
    
    void update(EntityID entityId, const CAABB& newBounds) override {
        // TODO: Rehash if entity moved to different buckets
    }
    
    void clear() override {
        // TODO: Clear hash table
    }
    
    std::vector<EntityID> query(const CAABB& region) const override {
        // TODO: Hash region and check relevant buckets
        return {};
    }
    
    std::vector<CollisionPair> findCollisions() const override {
        // TODO: Check all non-empty buckets for collisions
        return {};
    }
    
    std::vector<EntityID> queryRadius(const glm::vec3& point, float radius) const override {
        // TODO: Hash point with radius and check buckets
        return {};
    }
    
    void getStatistics(PartitionStats& stats) const override {
        // TODO: Calculate hash table statistics
        stats.reset();
    }
    
    const char* getStrategyName() const override {
        return "SpatialHash (TODO)";
    }
    
    bool isValid() const override {
        // TODO: Validate hash table state
        return true;
    }
};

// ============================================================================
// Factory Implementation
// ============================================================================

std::unique_ptr<SpatialPartitionStrategy> createSpatialPartition(
    PartitionType type,
    const glm::vec3& worldMin,
    const glm::vec3& worldMax,
    float cellSize) {
    
    switch (type) {
        case PartitionType::UNIFORM_GRID:
            return std::make_unique<UniformGridStrategy>(worldMin, worldMax, cellSize);
            
        case PartitionType::QUADTREE:
            return std::make_unique<QuadtreeStrategy>(worldMin, worldMax);
            
        case PartitionType::OCTREE:
            // TODO: Implement OctreeStrategy (3D version of Quadtree)
            std::cout << "TODO: Octree not yet implemented, falling back to UniformGrid" << std::endl;
            return std::make_unique<UniformGridStrategy>(worldMin, worldMax, cellSize);
            
        case PartitionType::SPATIAL_HASH:
            return std::make_unique<SpatialHashStrategy>(cellSize);
            
        default:
            std::cout << "Unknown partition type, falling back to UniformGrid" << std::endl;
            return std::make_unique<UniformGridStrategy>(worldMin, worldMax, cellSize);
    }
}