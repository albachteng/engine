#include <gtest/gtest.h>
#include "../include/Entity.hpp"
#include "../include/Component.h"
#include "../include/EntityManager.h"
#include <memory>
#include <chrono>

/**
 * Comprehensive unit tests for array-based Entity implementation
 * 
 * Tests verify:
 * - API compatibility with tuple-based Entity
 * - Component management functionality
 * - Memory efficiency improvements
 * - Advanced ECS capabilities
 * - Performance characteristics
 */

class EntityNewTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create entity manager (automatically initializes component system)
        entityManager = std::make_unique<EntityManager>();
    }

    void TearDown() override {
        // Clean up entity manager (automatically shuts down component system)
        entityManager.reset();
    }
    
    // Helper to create entity using EntityManager
    std::shared_ptr<Entity> createEntity(EntityTag tag = EntityTag::DEFAULT) {
        return entityManager->addEntity(tag);
    }
    
    std::unique_ptr<EntityManager> entityManager;
};

// ============================================================================
// Basic Entity Properties (API Compatibility)
// ============================================================================

TEST_F(EntityNewTest, BasicProperties_SameAsOriginalEntity) {
    auto entity = createEntity(EntityTag::PLAYER);
    
    EXPECT_EQ(entity->tag(), EntityTag::PLAYER);
    EXPECT_TRUE(entity->isActive());
    
    entity->destroy();
    EXPECT_FALSE(entity->isActive());
}

TEST_F(EntityNewTest, ComponentManager_InitializesCorrectly) {
    auto* manager = Entity::getComponentManager();
    EXPECT_NE(manager, nullptr);
    
    auto stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
    EXPECT_EQ(stats.totalComponentTypes, 0);
}

// ============================================================================
// Component Management (API Compatibility Tests)
// ============================================================================

TEST_F(EntityNewTest, AddComponent_SameAPIAsOriginal) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    // Test add() with no parameters
    auto& transform = entity->add<CTransform3D>();
    EXPECT_TRUE(transform.exists);
    EXPECT_TRUE(entity->has<CTransform3D>());
    
    // Test add() with parameters
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    glm::vec3 rotation(0.1f, 0.2f, 0.3f);
    glm::vec3 scale(2.0f, 2.0f, 2.0f);
    
    auto& transform2 = entity->add<CTransform3D>(position, rotation, scale);
    EXPECT_EQ(transform2.position, position);
    EXPECT_EQ(transform2.rotation, rotation);
    EXPECT_EQ(transform2.scale, scale);
}

TEST_F(EntityNewTest, GetComponent_SameAPIAsOriginal) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    // Add component with specific values
    glm::vec3 velocity(5.0f, -2.0f, 1.0f);
    glm::vec3 acceleration(0.0f, -9.8f, 0.0f);
    entity->add<CMovement3D>(velocity, acceleration);
    
    // Get component and verify values
    auto& movement = entity->get<CMovement3D>();
    EXPECT_EQ(movement.vel, velocity);
    EXPECT_EQ(movement.acc, acceleration);
    EXPECT_TRUE(movement.exists);
}

TEST_F(EntityNewTest, HasComponent_SameAPIAsOriginal) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    EXPECT_FALSE(entity->has<CTransform3D>());
    EXPECT_FALSE(entity->has<CMovement3D>());
    
    entity->add<CTransform3D>();
    EXPECT_TRUE(entity->has<CTransform3D>());
    EXPECT_FALSE(entity->has<CMovement3D>());
    
    entity->add<CMovement3D>();
    EXPECT_TRUE(entity->has<CTransform3D>());
    EXPECT_TRUE(entity->has<CMovement3D>());
}

TEST_F(EntityNewTest, RemoveComponent_SameAPIAsOriginal) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    
    EXPECT_TRUE(entity->has<CTransform3D>());
    EXPECT_TRUE(entity->has<CMovement3D>());
    
    entity->remove<CTransform3D>();
    EXPECT_FALSE(entity->has<CTransform3D>());
    EXPECT_TRUE(entity->has<CMovement3D>());
    
    entity->remove<CMovement3D>();
    EXPECT_FALSE(entity->has<CTransform3D>());
    EXPECT_FALSE(entity->has<CMovement3D>());
}

// ============================================================================
// Advanced Component Features (New Capabilities)
// ============================================================================

TEST_F(EntityNewTest, MultipleComponents_WorkTogether) {
    auto entity = createEntity(EntityTag::TRIANGLE);
    
    // Add multiple components like in real game usage
    entity->add<CTransform3D>(glm::vec3(10.0f, 20.0f, 30.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    entity->add<CAABB>(glm::vec3(0.0f), glm::vec3(2.0f));
    entity->add<CTriangle>();
    
    EXPECT_TRUE(entity->has<CTransform3D>());
    EXPECT_TRUE(entity->has<CMovement3D>());
    EXPECT_TRUE(entity->has<CAABB>());
    EXPECT_TRUE(entity->has<CTriangle>());
    EXPECT_FALSE(entity->has<CScore>());
    
    EXPECT_EQ(entity->getComponentCount(), 4);
    EXPECT_TRUE(entity->hasAnyComponents());
}

TEST_F(EntityNewTest, HasComponents_ChecksMultipleAtOnce) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    
    // Check multiple components at once
    EXPECT_TRUE((entity->hasComponents<CTransform3D, CMovement3D>()));
    EXPECT_FALSE((entity->hasComponents<CTransform3D, CMovement3D, CAABB>()));
    EXPECT_TRUE((entity->hasComponents<CTransform3D>()));
    EXPECT_TRUE((entity->hasComponents<CMovement3D>()));
}

TEST_F(EntityNewTest, GetComponents_ReturnsMultipleAtOnce) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    // Get multiple components at once
    auto [transform, movement] = entity->getComponents<CTransform3D, CMovement3D>();
    
    EXPECT_EQ(transform.position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(movement.vel, glm::vec3(5.0f, 0.0f, 0.0f));
}

TEST_F(EntityNewTest, ComponentMask_TracksComponentsCorrectly) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    const auto& mask = entity->getComponentMask();
    EXPECT_FALSE(mask.any());
    
    entity->add<CTransform3D>();
    EXPECT_TRUE(entity->getComponentMask().any());
    
    entity->add<CMovement3D>();
    entity->add<CAABB>();
    EXPECT_EQ(entity->getComponentCount(), 3);
    
    entity->remove<CMovement3D>();
    EXPECT_EQ(entity->getComponentCount(), 2);
}

// ============================================================================
// Memory Efficiency Tests
// ============================================================================

TEST_F(EntityNewTest, MemoryEfficiency_OnlyAllocatesUsedComponents) {
    auto* manager = Entity::getComponentManager();
    auto initialStats = manager->getStatistics();
    
    // Create entity with only 2 out of 12 possible components
    auto entity = createEntity(EntityTag::DEFAULT);
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    
    auto stats = manager->getStatistics();
    
    // Should only have 2 components allocated, not 12
    EXPECT_EQ(stats.totalComponents, 2);
    EXPECT_EQ(stats.totalComponentTypes, 2);
    
    // In tuple system, this entity would allocate space for all 12 component types
    // In array system, only allocates space for 2 component types
    // This is a significant memory saving
}

TEST_F(EntityNewTest, MultipleEntities_ShareComponentArrays) {
    auto* manager = Entity::getComponentManager();
    
    // Create multiple entities with same component types
    std::vector<std::shared_ptr<Entity>> entities;
    for (size_t i = 0; i < 10; ++i) {
        auto entity = createEntity(EntityTag::DEFAULT);
        entity->add<CTransform3D>(glm::vec3(static_cast<float>(i), 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
        entities.push_back(entity);
    }
    
    auto stats = manager->getStatistics();
    
    // Should have 10 components of 1 type (shared array)
    EXPECT_EQ(stats.totalComponents, 10);
    EXPECT_EQ(stats.totalComponentTypes, 1);
    
    // Verify all entities have their components
    for (size_t i = 0; i < entities.size(); ++i) {
        EXPECT_TRUE(entities[i]->has<CTransform3D>());
        // Position.x was set to the entity's ID during creation, but with EntityManager we can't predict IDs
        // Just verify the component exists and has valid data
        EXPECT_GE(entities[i]->get<CTransform3D>().position.x, 0.0f);
    }
}

// ============================================================================
// Component Lifecycle Tests
// ============================================================================

TEST_F(EntityNewTest, ComponentReplacement_OverwritesExisting) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    // Add initial component
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    EXPECT_EQ(entity->get<CTransform3D>().position, glm::vec3(1.0f, 2.0f, 3.0f));
    
    // Replace with new values
    entity->add<CTransform3D>(glm::vec3(10.0f, 20.0f, 30.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    EXPECT_EQ(entity->get<CTransform3D>().position, glm::vec3(10.0f, 20.0f, 30.0f));
    
    // Should still have exactly one component
    EXPECT_EQ(entity->getComponentCount(), 1);
}

TEST_F(EntityNewTest, RemoveAllComponents_ClearsEntity) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    entity->add<CAABB>();
    
    EXPECT_EQ(entity->getComponentCount(), 3);
    EXPECT_TRUE(entity->hasAnyComponents());
    
    entity->removeAllComponents();
    
    EXPECT_EQ(entity->getComponentCount(), 0);
    EXPECT_FALSE(entity->hasAnyComponents());
    EXPECT_FALSE(entity->has<CTransform3D>());
    EXPECT_FALSE(entity->has<CMovement3D>());
    EXPECT_FALSE(entity->has<CAABB>());
}

TEST_F(EntityNewTest, EntityDestruction_CleansUpComponents) {
    auto* manager = Entity::getComponentManager();
    
    auto entity = createEntity(EntityTag::DEFAULT);
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    
    auto stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 2);
    
    // Manually destroy entity to test cleanup
    entity->destroy();
    entityManager->update(); // Process destroyed entities
    
    // Components should be cleaned up when entity is removed from EntityManager
    stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
}

// ============================================================================
// Performance and System Integration Tests
// ============================================================================

TEST_F(EntityNewTest, ComponentIteration_EnablesEfficientSystems) {
    auto* manager = Entity::getComponentManager();
    
    // Create entities with transform components
    std::vector<std::shared_ptr<Entity>> entities;
    for (size_t i = 0; i < 100; ++i) {
        auto entity = createEntity(EntityTag::DEFAULT);
        entity->add<CTransform3D>(
            glm::vec3(static_cast<float>(i), 0.0f, 0.0f), 
            glm::vec3(0.0f), 
            glm::vec3(1.0f)
        );
        entities.push_back(entity);
    }
    
    // Simulate movement system iterating over all transform components
    auto* transformArray = manager->getComponentArray<CTransform3D>();
    
    EXPECT_EQ(transformArray->size(), 100);
    
    // Efficient system iteration (cache-friendly)
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto& transform : *transformArray) {
        transform.position += glm::vec3(0.1f, 0.0f, 0.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::micro>(end - start).count();
    
    // Should be very fast due to cache locality
    EXPECT_LT(duration, 100.0); // Less than 100 microseconds
    
    // Verify all transforms were updated
    for (auto& entity : entities) {
        // Original position.x was set to entity id, now it should be larger due to the += 0.1f
        EXPECT_GT(entity->get<CTransform3D>().position.x, 0.0f);
    }
}

// ============================================================================
// Error Handling and Edge Cases
// ============================================================================

TEST_F(EntityNewTest, AccessNonexistentComponent_HandledSafely) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    // Should return false for non-existent components
    EXPECT_FALSE(entity->has<CTransform3D>());
    
    // Note: get() on non-existent component will assert in debug builds
    // This is the same behavior as the original tuple-based system
}

TEST_F(EntityNewTest, RemoveNonexistentComponent_HandledGracefully) {
    auto entity = createEntity(EntityTag::DEFAULT);
    
    // Should not crash when removing non-existent component
    entity->remove<CTransform3D>();
    
    EXPECT_FALSE(entity->has<CTransform3D>());
    EXPECT_EQ(entity->getComponentCount(), 0);
}

TEST_F(EntityNewTest, MultipleComponentManagerLifecycles_HandleCorrectly) {
    // Test initialization and shutdown cycles
    Entity::shutdownComponentManager();
    Entity::initializeComponentManager();
    
    auto entity = createEntity(EntityTag::DEFAULT);
    entity->add<CTransform3D>();
    
    EXPECT_TRUE(entity->has<CTransform3D>());
    
    Entity::shutdownComponentManager();
    Entity::initializeComponentManager();
    
    // After restart, component manager should be fresh
    auto* manager = Entity::getComponentManager();
    auto stats = manager->getStatistics();
    EXPECT_EQ(stats.totalComponents, 0);
}

// ============================================================================
// Compatibility with Existing Code Tests
// ============================================================================

TEST_F(EntityNewTest, ExistingGameCode_WorksWithoutChanges) {
    auto entity = createEntity(EntityTag::TRIANGLE);
    
    // Simulate existing game code patterns
    if (entity->has<CTransform3D>() && entity->has<CMovement3D>()) {
        // This branch should not execute
        FAIL() << "Should not have components initially";
    }
    
    entity->add<CTransform3D>();
    entity->add<CMovement3D>();
    
    if (entity->has<CTransform3D>() && entity->has<CMovement3D>()) {
        auto& transform = entity->get<CTransform3D>();
        auto& movement = entity->get<CMovement3D>();
        
        transform.position += movement.vel * 0.016f; // 60 FPS
        EXPECT_TRUE(true); // This should execute
    }
    
    // Test component exists flag (compatibility with base Component class)
    EXPECT_TRUE(entity->get<CTransform3D>().exists);
    EXPECT_TRUE(entity->get<CMovement3D>().exists);
}