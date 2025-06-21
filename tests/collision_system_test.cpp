#include <gtest/gtest.h>
#include "../include/CollisionSystem.hpp"
#include "../include/EntityManager.h"
#include "../include/Component.h"
#include <memory>
#include <chrono>
#include <algorithm>

/**
 * Comprehensive unit tests for CollisionSystem and SpatialPartitioning
 * 
 * Tests cover:
 * - Basic collision detection functionality
 * - Performance improvements over O(N²) detection
 * - Edge cases and boundary conditions
 * - Integration with EntityManager
 * - Multiple spatial partitioning strategies
 */
class CollisionSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager.clear();
        
        // Initialize collision system with small world for testing
        collisionSystem = std::make_unique<CollisionSystem>(
            PartitionType::UNIFORM_GRID,
            glm::vec3(-50.0f, -50.0f, -50.0f),
            glm::vec3(50.0f, 50.0f, 50.0f),
            5.0f // Small cell size for testing
        );
    }

    void TearDown() override {
        manager.clear();
        collisionSystem->clear();
    }

    // Helper function to create entity with CAABB and CTransform3D
    std::shared_ptr<Entity> createEntity(const glm::vec3& position, 
                                       const glm::vec3& halfExtents,
                                       EntityTag tag = EntityTag::DEFAULT) {
        auto entity = manager.addEntity(tag);
        
        // Add transform component
        entity->add<CTransform3D>(position, glm::vec3(0.0f), glm::vec3(1.0f));
        
        // Add AABB component (center + halfExtents)
        entity->add<CAABB>(glm::vec3(0.0f), halfExtents);
        
        manager.update(); // Process addition
        return entity;
    }

    EntityManager manager;
    std::unique_ptr<CollisionSystem> collisionSystem;
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(CollisionSystemTest, Initialization_CreatesValidSystem) {
    EXPECT_TRUE(collisionSystem->isValid());
    
    // Test statistics for empty system
    auto stats = collisionSystem->getStatistics();
    EXPECT_EQ(stats.totalEntities, 0);
    EXPECT_GT(stats.totalNodes, 0); // Should have grid cells
}

TEST_F(CollisionSystemTest, SingleEntity_NoCollisions) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_TRUE(collisions.empty());
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 1);
}

TEST_F(CollisionSystemTest, TwoSeparateEntities_NoCollisions) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity2 = createEntity(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_TRUE(collisions.empty());
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 2);
}

TEST_F(CollisionSystemTest, TwoOverlappingEntities_DetectsCollision) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity2 = createEntity(glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(1.0f)); // Overlapping
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    ASSERT_EQ(collisions.size(), 1);
    
    // Check that collision pair contains both entities
    auto [id1, id2] = collisions[0];
    EXPECT_TRUE((id1 == entity1->id() && id2 == entity2->id()) ||
                (id1 == entity2->id() && id2 == entity1->id()));
}

TEST_F(CollisionSystemTest, TouchingEntities_NoCollision) {
    // Entities exactly touching (edge case) - should NOT be detected as collision
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity2 = createEntity(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f)); // Touching at x=1
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    // AABB intersection logic: (a.max.x > b.min.x) means 1.0 > 1.0 = false
    // So touching entities should NOT collide
    EXPECT_EQ(collisions.size(), 0);
}

// ============================================================================
// Multi-Entity Collision Tests
// ============================================================================

TEST_F(CollisionSystemTest, MultipleEntities_DetectsAllCollisions) {
    // Create cluster of 4 overlapping entities
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f));
    auto entity2 = createEntity(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.5f));
    auto entity3 = createEntity(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.5f));
    auto entity4 = createEntity(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.5f));
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    // Should detect 6 collision pairs (4 choose 2)
    EXPECT_EQ(collisions.size(), 6);
}

TEST_F(CollisionSystemTest, ChainCollisions_DetectsCorrectPairs) {
    // Create chain: A-B-C where A overlaps B, B overlaps C, but A doesn't overlap C
    auto entityA = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entityB = createEntity(glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(1.0f)); // Overlaps A
    auto entityC = createEntity(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f)); // Overlaps B
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_EQ(collisions.size(), 2); // A-B and B-C
}

// ============================================================================
// Query Interface Tests
// ============================================================================

TEST_F(CollisionSystemTest, QueryRegion_FindsEntitiesInRegion) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity2 = createEntity(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity3 = createEntity(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    
    // Query region around origin
    CAABB queryRegion;
    queryRegion.min = glm::vec3(-2.0f, -2.0f, -2.0f);
    queryRegion.max = glm::vec3(3.0f, 3.0f, 3.0f);
    
    auto results = collisionSystem->queryRegion(queryRegion, manager);
    
    EXPECT_EQ(results.size(), 2); // entity1 and entity3 should be found
    
    // Verify correct entities were found
    bool foundEntity1 = false, foundEntity3 = false;
    for (auto& entity : results) {
        if (entity->id() == entity1->id()) foundEntity1 = true;
        if (entity->id() == entity3->id()) foundEntity3 = true;
    }
    EXPECT_TRUE(foundEntity1);
    EXPECT_TRUE(foundEntity3);
}

TEST_F(CollisionSystemTest, QueryRadius_FindsEntitiesWithinRadius) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f));
    auto entity2 = createEntity(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.5f)); // 3 units away
    auto entity3 = createEntity(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.5f)); // ~1.4 units away
    
    collisionSystem->updateEntities(manager);
    
    // Query radius of 2.0 from origin
    auto results = collisionSystem->queryRadius(glm::vec3(0.0f), 2.0f, manager);
    
    EXPECT_EQ(results.size(), 2); // entity1 and entity3 should be found
}

TEST_F(CollisionSystemTest, FindCollisionsFor_FindsSpecificEntityCollisions) {
    auto targetEntity = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f));
    auto collider1 = createEntity(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto collider2 = createEntity(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f));
    auto nonCollider = createEntity(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisionsFor(targetEntity, manager);
    
    EXPECT_EQ(collisions.size(), 2); // Should collide with collider1 and collider2
}

// ============================================================================
// Performance and Scalability Tests
// ============================================================================

TEST_F(CollisionSystemTest, LargeNumberOfEntities_PerformsEfficiently) {
    const int numEntities = 100;
    
    // Create entities in a grid pattern with some overlaps
    for (int i = 0; i < numEntities; ++i) {
        float x = (i % 10) * 2.0f;
        float y = (i / 10) * 2.0f;
        createEntity(glm::vec3(x, y, 0.0f), glm::vec3(1.1f)); // Slightly overlapping
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Performance should be reasonable (< 50ms for 100 entities)
    EXPECT_LT(duration, 50.0);
    EXPECT_GT(collisions.size(), 0); // Should find some collisions
    
    auto stats = collisionSystem->getStatistics();
    EXPECT_EQ(stats.totalEntities, numEntities);
    EXPECT_LT(stats.lastQueryTimeMs, 50.0);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(CollisionSystemTest, EmptyEntityManager_HandlesGracefully) {
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_TRUE(collisions.empty());
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 0);
}

TEST_F(CollisionSystemTest, EntitiesWithoutAABB_IgnoredSafely) {
    auto entity1 = manager.addEntity(EntityTag::DEFAULT);
    entity1->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    // No CAABB component
    
    auto entity2 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    manager.update();
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_TRUE(collisions.empty()); // No collisions since entity1 has no AABB
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 1); // Only entity2 tracked
}

TEST_F(CollisionSystemTest, EntitiesOutsideWorldBounds_HandledCorrectly) {
    // Create entity outside world bounds
    auto outsideEntity = createEntity(glm::vec3(100.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto insideEntity = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    auto collisions = collisionSystem->findCollisions();
    
    EXPECT_TRUE(collisions.empty()); // Outside entity should be ignored
    
    auto stats = collisionSystem->getStatistics();
    EXPECT_EQ(stats.totalEntities, 1); // Only inside entity should be tracked
}

TEST_F(CollisionSystemTest, ClearSystem_RemovesAllEntities) {
    createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    createEntity(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 2);
    
    collisionSystem->clear();
    EXPECT_EQ(collisionSystem->getStatistics().totalEntities, 0);
    
    auto collisions = collisionSystem->findCollisions();
    EXPECT_TRUE(collisions.empty());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(CollisionSystemTest, EntityTagFiltering_WorksCorrectly) {
    auto triangle1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), EntityTag::TRIANGLE);
    auto triangle2 = createEntity(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f), EntityTag::TRIANGLE);
    auto player = createEntity(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(1.0f), EntityTag::PLAYER);
    
    collisionSystem->updateEntities(manager);
    auto allCollisions = collisionSystem->findCollisions();
    
    // Should detect 3 collision pairs: triangle1-triangle2, triangle1-player, triangle2-player
    EXPECT_EQ(allCollisions.size(), 3);
    
    // Test finding collisions for specific entity
    auto playerCollisions = collisionSystem->findCollisionsFor(player, manager);
    EXPECT_EQ(playerCollisions.size(), 2); // Should collide with both triangles
}

TEST_F(CollisionSystemTest, UpdateAfterEntityMovement_DetectsNewCollisions) {
    auto entity1 = createEntity(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    auto entity2 = createEntity(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    collisionSystem->updateEntities(manager);
    auto initialCollisions = collisionSystem->findCollisions();
    EXPECT_TRUE(initialCollisions.empty());
    
    // Move entity2 closer
    entity2->get<CTransform3D>().position = glm::vec3(1.5f, 0.0f, 0.0f);
    
    collisionSystem->updateEntities(manager);
    auto newCollisions = collisionSystem->findCollisions();
    EXPECT_EQ(newCollisions.size(), 1); // Now they should collide
}

// ============================================================================
// AABBUtils Tests
// ============================================================================

TEST_F(CollisionSystemTest, AABBUtils_IntersectionDetection) {
    CAABB box1;
    box1.min = glm::vec3(0.0f, 0.0f, 0.0f);
    box1.max = glm::vec3(2.0f, 2.0f, 2.0f);
    
    CAABB box2;
    box2.min = glm::vec3(1.0f, 1.0f, 1.0f);
    box2.max = glm::vec3(3.0f, 3.0f, 3.0f);
    
    CAABB box3;
    box3.min = glm::vec3(5.0f, 5.0f, 5.0f);
    box3.max = glm::vec3(7.0f, 7.0f, 7.0f);
    
    EXPECT_TRUE(AABBUtils::intersects(box1, box2));  // Overlapping
    EXPECT_FALSE(AABBUtils::intersects(box1, box3)); // Separate
    EXPECT_TRUE(AABBUtils::intersects(box1, box1));  // Self-intersection
}

TEST_F(CollisionSystemTest, AABBUtils_GetWorldAABB) {
    auto entity = createEntity(glm::vec3(5.0f, 3.0f, 1.0f), glm::vec3(2.0f, 1.0f, 0.5f));
    
    CAABB worldAABB = AABBUtils::getWorldAABB(entity);
    
    // World AABB should be local AABB + transform position
    EXPECT_EQ(worldAABB.min, glm::vec3(3.0f, 2.0f, 0.5f)); // (5,3,1) + (-2,-1,-0.5)
    EXPECT_EQ(worldAABB.max, glm::vec3(7.0f, 4.0f, 1.5f)); // (5,3,1) + (2,1,0.5)
}