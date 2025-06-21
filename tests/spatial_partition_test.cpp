#include <gtest/gtest.h>
#include "../include/SpatialPartition.hpp"
#include <memory>
#include <set>
#include <algorithm>

/**
 * Unit tests for SpatialPartition strategies and algorithms
 * 
 * Tests the low-level spatial partitioning implementations:
 * - UniformGrid algorithm correctness
 * - Performance characteristics
 * - Edge cases and boundary conditions
 * - Factory pattern functionality
 */
class SpatialPartitionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create small test world
        worldMin = glm::vec3(-10.0f, -10.0f, -10.0f);
        worldMax = glm::vec3(10.0f, 10.0f, 10.0f);
        cellSize = 2.0f;
        
        partition = createSpatialPartition(
            PartitionType::UNIFORM_GRID, 
            worldMin, 
            worldMax, 
            cellSize
        );
    }

    void TearDown() override {
        if (partition) {
            partition->clear();
        }
    }

    // Helper to create CAABB
    CAABB createAABB(const glm::vec3& center, const glm::vec3& halfExtents) {
        CAABB aabb;
        aabb.min = center - halfExtents;
        aabb.max = center + halfExtents;
        return aabb;
    }

    glm::vec3 worldMin, worldMax;
    float cellSize;
    std::unique_ptr<SpatialPartitionStrategy> partition;
};

// ============================================================================
// Factory Pattern Tests
// ============================================================================

TEST_F(SpatialPartitionTest, Factory_CreatesUniformGrid) {
    auto grid = createSpatialPartition(PartitionType::UNIFORM_GRID, worldMin, worldMax, cellSize);
    
    ASSERT_NE(grid, nullptr);
    EXPECT_STREQ(grid->getStrategyName(), "UniformGrid");
    EXPECT_TRUE(grid->isValid());
}

TEST_F(SpatialPartitionTest, Factory_CreatesQuadtreeStub) {
    auto quadtree = createSpatialPartition(PartitionType::QUADTREE, worldMin, worldMax, cellSize);
    
    ASSERT_NE(quadtree, nullptr);
    EXPECT_STREQ(quadtree->getStrategyName(), "Quadtree (TODO)");
}

TEST_F(SpatialPartitionTest, Factory_CreatesSpatialHashStub) {
    auto hash = createSpatialPartition(PartitionType::SPATIAL_HASH, worldMin, worldMax, cellSize);
    
    ASSERT_NE(hash, nullptr);
    EXPECT_STREQ(hash->getStrategyName(), "SpatialHash (TODO)");
}

TEST_F(SpatialPartitionTest, Factory_HandlesUnknownType) {
    // Cast to force unknown type
    auto unknown = createSpatialPartition(static_cast<PartitionType>(999), worldMin, worldMax, cellSize);
    
    ASSERT_NE(unknown, nullptr);
    EXPECT_STREQ(unknown->getStrategyName(), "UniformGrid"); // Should fallback
}

// ============================================================================
// UniformGrid Basic Operations
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_EmptyPartition) {
    EXPECT_TRUE(partition->isValid());
    
    auto collisions = partition->findCollisions();
    EXPECT_TRUE(collisions.empty());
    
    PartitionStats stats;
    partition->getStatistics(stats);
    EXPECT_EQ(stats.totalEntities, 0);
    EXPECT_GT(stats.totalNodes, 0); // Should have grid cells
}

TEST_F(SpatialPartitionTest, UniformGrid_InsertSingleEntity) {
    EntityID entityId = 42;
    CAABB bounds = createAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    
    partition->insert(entityId, bounds);
    
    EXPECT_TRUE(partition->isValid());
    
    PartitionStats stats;
    partition->getStatistics(stats);
    EXPECT_EQ(stats.totalEntities, 1);
}

TEST_F(SpatialPartitionTest, UniformGrid_InsertAndQuery) {
    EntityID entityId = 123;
    glm::vec3 position(2.0f, 3.0f, 1.0f);
    CAABB bounds = createAABB(position, glm::vec3(0.5f, 0.5f, 0.5f));
    
    partition->insert(entityId, bounds);
    
    // Query region that should contain the entity
    CAABB queryRegion = createAABB(position, glm::vec3(1.0f, 1.0f, 1.0f));
    auto results = partition->query(queryRegion);
    
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entityId);
}

TEST_F(SpatialPartitionTest, UniformGrid_QueryEmpty) {
    // Query empty region
    CAABB queryRegion = createAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    auto results = partition->query(queryRegion);
    
    EXPECT_TRUE(results.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_QueryOutsideBounds) {
    EntityID entityId = 456;
    CAABB bounds = createAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    partition->insert(entityId, bounds);
    
    // Query far outside world bounds
    CAABB queryRegion = createAABB(glm::vec3(100.0f, 100.0f, 100.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    auto results = partition->query(queryRegion);
    
    EXPECT_TRUE(results.empty());
}

// ============================================================================
// Multi-Entity Operations
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_MultipleEntitiesSameCell) {
    // Insert multiple entities in the same grid cell
    EntityID id1 = 1, id2 = 2, id3 = 3;
    glm::vec3 basePos(0.0f, 0.0f, 0.0f);
    
    partition->insert(id1, createAABB(basePos + glm::vec3(0.1f, 0.0f, 0.0f), glm::vec3(0.2f)));
    partition->insert(id2, createAABB(basePos + glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.2f)));
    partition->insert(id3, createAABB(basePos + glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(0.2f)));
    
    CAABB queryRegion = createAABB(basePos, glm::vec3(1.0f));
    auto results = partition->query(queryRegion);
    
    EXPECT_EQ(results.size(), 3);
    
    // All entities should be found
    EXPECT_TRUE(std::find(results.begin(), results.end(), id1) != results.end());
    EXPECT_TRUE(std::find(results.begin(), results.end(), id2) != results.end());
    EXPECT_TRUE(std::find(results.begin(), results.end(), id3) != results.end());
}

TEST_F(SpatialPartitionTest, UniformGrid_EntitiesInDifferentCells) {
    EntityID id1 = 1, id2 = 2;
    
    // Place entities in clearly different cells
    partition->insert(id1, createAABB(glm::vec3(-5.0f, -5.0f, 0.0f), glm::vec3(0.5f)));
    partition->insert(id2, createAABB(glm::vec3(5.0f, 5.0f, 0.0f), glm::vec3(0.5f)));
    
    // Query first entity's region
    auto results1 = partition->query(createAABB(glm::vec3(-5.0f, -5.0f, 0.0f), glm::vec3(1.0f)));
    ASSERT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0], id1);
    
    // Query second entity's region
    auto results2 = partition->query(createAABB(glm::vec3(5.0f, 5.0f, 0.0f), glm::vec3(1.0f)));
    ASSERT_EQ(results2.size(), 1);
    EXPECT_EQ(results2[0], id2);
}

// ============================================================================
// Collision Detection Tests
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_FindCollisionsEmpty) {
    auto collisions = partition->findCollisions();
    EXPECT_TRUE(collisions.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_FindCollisionsSingleEntity) {
    partition->insert(1, createAABB(glm::vec3(0.0f), glm::vec3(1.0f)));
    
    auto collisions = partition->findCollisions();
    EXPECT_TRUE(collisions.empty()); // Single entity can't collide with itself
}

TEST_F(SpatialPartitionTest, UniformGrid_FindCollisionsOverlapping) {
    // Create two overlapping entities
    partition->insert(1, createAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f)));
    partition->insert(2, createAABB(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f)));
    
    auto collisions = partition->findCollisions();
    
    ASSERT_EQ(collisions.size(), 1);
    auto [id1, id2] = collisions[0];
    EXPECT_TRUE((id1 == 1 && id2 == 2) || (id1 == 2 && id2 == 1));
}

TEST_F(SpatialPartitionTest, UniformGrid_FindCollisionsNonOverlapping) {
    // Create two separate entities
    partition->insert(1, createAABB(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(1.0f)));
    partition->insert(2, createAABB(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f)));
    
    auto collisions = partition->findCollisions();
    EXPECT_TRUE(collisions.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_FindCollisionsMultiple) {
    // Create cluster of 4 overlapping entities
    partition->insert(1, createAABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f)));
    partition->insert(2, createAABB(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.5f)));
    partition->insert(3, createAABB(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.5f)));
    partition->insert(4, createAABB(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.5f)));
    
    auto collisions = partition->findCollisions();
    
    // Should find 6 collision pairs (4 choose 2)
    EXPECT_EQ(collisions.size(), 6);
    
    // Verify no duplicate pairs
    std::set<std::pair<EntityID, EntityID>> uniquePairs;
    for (auto [id1, id2] : collisions) {
        auto normalizedPair = std::make_pair(std::min(id1, id2), std::max(id1, id2));
        EXPECT_TRUE(uniquePairs.insert(normalizedPair).second) << "Duplicate collision pair found";
    }
}

// ============================================================================
// Update and Remove Operations
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_RemoveEntity) {
    EntityID entityId = 789;
    partition->insert(entityId, createAABB(glm::vec3(0.0f), glm::vec3(1.0f)));
    
    // Verify entity is there
    auto results1 = partition->query(createAABB(glm::vec3(0.0f), glm::vec3(2.0f)));
    EXPECT_EQ(results1.size(), 1);
    
    // Remove entity
    partition->remove(entityId);
    
    // Verify entity is gone
    auto results2 = partition->query(createAABB(glm::vec3(0.0f), glm::vec3(2.0f)));
    EXPECT_TRUE(results2.empty());
    
    EXPECT_TRUE(partition->isValid());
}

TEST_F(SpatialPartitionTest, UniformGrid_UpdateEntityPosition) {
    EntityID entityId = 101;
    CAABB initialBounds = createAABB(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    CAABB newBounds = createAABB(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    partition->insert(entityId, initialBounds);
    
    // Verify entity is in initial position
    auto results1 = partition->query(createAABB(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(2.0f)));
    EXPECT_EQ(results1.size(), 1);
    
    // Update position
    partition->update(entityId, newBounds);
    
    // Verify entity moved to new position
    auto results2 = partition->query(createAABB(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(2.0f)));
    EXPECT_TRUE(results2.empty());
    
    auto results3 = partition->query(createAABB(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(2.0f)));
    EXPECT_EQ(results3.size(), 1);
    EXPECT_EQ(results3[0], entityId);
}

TEST_F(SpatialPartitionTest, UniformGrid_UpdateNonExistentEntity) {
    // Update entity that was never inserted (should work gracefully)
    EntityID entityId = 999;
    CAABB bounds = createAABB(glm::vec3(0.0f), glm::vec3(1.0f));
    
    partition->update(entityId, bounds); // Should not crash
    
    // Should effectively insert the entity
    auto results = partition->query(createAABB(glm::vec3(0.0f), glm::vec3(2.0f)));
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entityId);
}

// ============================================================================
// Radius Query Tests
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_QueryRadiusEmpty) {
    auto results = partition->queryRadius(glm::vec3(0.0f), 5.0f);
    EXPECT_TRUE(results.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_QueryRadiusSingle) {
    EntityID entityId = 202;
    partition->insert(entityId, createAABB(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.5f)));
    
    // Query from origin with radius that should include entity
    auto results1 = partition->queryRadius(glm::vec3(0.0f), 3.0f);
    EXPECT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0], entityId);
    
    // Query with radius that should exclude entity
    auto results2 = partition->queryRadius(glm::vec3(0.0f), 1.0f);
    EXPECT_TRUE(results2.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_QueryRadiusMultiple) {
    // Create entities at various distances from origin
    partition->insert(1, createAABB(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.1f))); // Distance 1
    partition->insert(2, createAABB(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.1f))); // Distance 3
    partition->insert(3, createAABB(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.1f))); // Distance 4
    partition->insert(4, createAABB(glm::vec3(7.0f, 0.0f, 0.0f), glm::vec3(0.1f))); // Distance 7
    
    auto results = partition->queryRadius(glm::vec3(0.0f), 3.5f);
    
    EXPECT_EQ(results.size(), 2); // Should find entities 1 and 2
    EXPECT_TRUE(std::find(results.begin(), results.end(), 1) != results.end());
    EXPECT_TRUE(std::find(results.begin(), results.end(), 2) != results.end());
}

// ============================================================================
// Clear and Statistics Tests
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_Clear) {
    // Add several entities
    partition->insert(1, createAABB(glm::vec3(0.0f), glm::vec3(1.0f)));
    partition->insert(2, createAABB(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f)));
    partition->insert(3, createAABB(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(1.0f)));
    
    PartitionStats stats1;
    partition->getStatistics(stats1);
    EXPECT_EQ(stats1.totalEntities, 3);
    
    // Clear partition
    partition->clear();
    
    PartitionStats stats2;
    partition->getStatistics(stats2);
    EXPECT_EQ(stats2.totalEntities, 0);
    
    // Verify no entities found
    auto results = partition->query(createAABB(glm::vec3(0.0f), glm::vec3(10.0f)));
    EXPECT_TRUE(results.empty());
    
    auto collisions = partition->findCollisions();
    EXPECT_TRUE(collisions.empty());
}

TEST_F(SpatialPartitionTest, UniformGrid_Statistics) {
    // Add entities and check statistics
    partition->insert(1, createAABB(glm::vec3(0.0f), glm::vec3(1.0f)));
    partition->insert(2, createAABB(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f)));
    
    PartitionStats stats;
    partition->getStatistics(stats);
    
    EXPECT_EQ(stats.totalEntities, 2);
    EXPECT_GT(stats.totalNodes, 0);
    EXPECT_EQ(stats.maxDepth, 1); // Grid has no depth
    EXPECT_GE(stats.emptyNodes, 0);
    
    // Test collision timing
    auto collisions = partition->findCollisions();
    partition->getStatistics(stats);
    EXPECT_GT(stats.lastQueryTimeMs, 0.0);
    EXPECT_GT(stats.totalCollisionChecks, 0);
}

// ============================================================================
// Boundary and Edge Cases
// ============================================================================

TEST_F(SpatialPartitionTest, UniformGrid_EntitySpanningMultipleCells) {
    // Create large entity that spans multiple grid cells
    EntityID entityId = 303;
    CAABB largeBounds = createAABB(glm::vec3(0.0f), glm::vec3(3.0f)); // Spans several 2x2 cells
    
    partition->insert(entityId, largeBounds);
    
    // Query different regions that should all contain this entity
    auto results1 = partition->query(createAABB(glm::vec3(-2.0f, -2.0f, 0.0f), glm::vec3(0.5f)));
    auto results2 = partition->query(createAABB(glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.5f)));
    auto results3 = partition->query(createAABB(glm::vec3(-2.0f, 2.0f, 0.0f), glm::vec3(0.5f)));
    
    EXPECT_EQ(results1.size(), 1);
    EXPECT_EQ(results2.size(), 1);
    EXPECT_EQ(results3.size(), 1);
    EXPECT_EQ(results1[0], entityId);
    EXPECT_EQ(results2[0], entityId);
    EXPECT_EQ(results3[0], entityId);
}

TEST_F(SpatialPartitionTest, UniformGrid_RemoveNonExistentEntity) {
    // Remove entity that was never added (should not crash)
    partition->remove(999);
    
    EXPECT_TRUE(partition->isValid());
    
    PartitionStats stats;
    partition->getStatistics(stats);
    EXPECT_EQ(stats.totalEntities, 0);
}

TEST_F(SpatialPartitionTest, UniformGrid_EntityAtWorldBoundary) {
    // Test entity exactly at world boundary
    EntityID entityId = 404;
    CAABB boundaryBounds = createAABB(worldMax, glm::vec3(0.1f)); // At max boundary
    
    partition->insert(entityId, boundaryBounds);
    
    auto results = partition->query(createAABB(worldMax, glm::vec3(1.0f)));
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], entityId);
}