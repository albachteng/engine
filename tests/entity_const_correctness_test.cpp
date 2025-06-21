#include <gtest/gtest.h>
#include "../include/Entity.hpp"
#include "../include/EntityManager.h"
#include "../include/Component.h"
#include <memory>

/**
 * Test const correctness for Entity component access
 * 
 * Verifies that:
 * - Const Entity objects can read components without compilation errors
 * - Const methods return const references
 * - Non-const methods return non-const references
 * - Component manager const overloads work correctly
 */

class EntityConstCorrectnessTest : public ::testing::Test {
protected:
    void SetUp() override {
        entityManager = std::make_unique<EntityManager>();
    }

    void TearDown() override {
        entityManager.reset();
    }
    
    std::shared_ptr<Entity> createEntity(EntityTag tag = EntityTag::DEFAULT) {
        return entityManager->addEntity(tag);
    }
    
    std::unique_ptr<EntityManager> entityManager;
};

// Test const Entity can read components
TEST_F(EntityConstCorrectnessTest, ConstEntity_CanReadComponents) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    // Create const reference to entity
    const Entity& constEntity = *entity;
    
    // Should be able to check for components
    EXPECT_TRUE(constEntity.has<CTransform3D>());
    EXPECT_TRUE(constEntity.has<CMovement3D>());
    EXPECT_FALSE(constEntity.has<CAABB>());
}

TEST_F(EntityConstCorrectnessTest, ConstEntity_CanGetConstComponents) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    
    const Entity& constEntity = *entity;
    
    // Should be able to get const component reference
    const CTransform3D& transform = constEntity.get<CTransform3D>();
    EXPECT_EQ(transform.position, glm::vec3(1.0f, 2.0f, 3.0f));
    
    // Verify component is actually const (this should compile)
    // transform.position = glm::vec3(0.0f); // This should NOT compile if uncommented
}

TEST_F(EntityConstCorrectnessTest, ConstEntity_CanUseAdvancedMethods) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    const Entity& constEntity = *entity;
    
    // Should be able to use hasComponents
    EXPECT_TRUE((constEntity.hasComponents<CTransform3D, CMovement3D>()));
    EXPECT_FALSE((constEntity.hasComponents<CTransform3D, CMovement3D, CAABB>()));
    
    // Should be able to get component mask
    const auto& mask = constEntity.getComponentMask();
    EXPECT_TRUE(mask.any());
    
    // Should be able to get component count
    EXPECT_EQ(constEntity.getComponentCount(), 2);
    EXPECT_TRUE(constEntity.hasAnyComponents());
}

TEST_F(EntityConstCorrectnessTest, ConstEntity_CanGetMultipleComponents) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    const Entity& constEntity = *entity;
    
    // Should be able to get multiple components at once (const version)
    auto [transform, movement] = constEntity.getComponents<CTransform3D, CMovement3D>();
    
    EXPECT_EQ(transform.position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(movement.vel, glm::vec3(5.0f, 0.0f, 0.0f));
    
    // Verify components are const references
    static_assert(std::is_const_v<std::remove_reference_t<decltype(transform)>>, 
                  "Transform should be const reference");
    static_assert(std::is_const_v<std::remove_reference_t<decltype(movement)>>, 
                  "Movement should be const reference");
}

TEST_F(EntityConstCorrectnessTest, NonConstEntity_CanModifyComponents) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    
    // Non-const entity should be able to modify components
    CTransform3D& transform = entity->get<CTransform3D>();
    transform.position = glm::vec3(10.0f, 20.0f, 30.0f);
    
    EXPECT_EQ(entity->get<CTransform3D>().position, glm::vec3(10.0f, 20.0f, 30.0f));
}

TEST_F(EntityConstCorrectnessTest, NonConstEntity_GetMultipleComponentsReturnsNonConst) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    // Non-const version should return non-const references
    auto [transform, movement] = entity->getComponents<CTransform3D, CMovement3D>();
    
    // Should be able to modify through non-const references
    transform.position = glm::vec3(100.0f, 200.0f, 300.0f);
    movement.vel = glm::vec3(10.0f, 20.0f, 30.0f);
    
    EXPECT_EQ(entity->get<CTransform3D>().position, glm::vec3(100.0f, 200.0f, 300.0f));
    EXPECT_EQ(entity->get<CMovement3D>().vel, glm::vec3(10.0f, 20.0f, 30.0f));
    
    // Verify components are non-const references
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(transform)>>, 
                  "Transform should be non-const reference");
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(movement)>>, 
                  "Movement should be non-const reference");
}

// Test that const ComponentManager works correctly
TEST_F(EntityConstCorrectnessTest, ConstComponentManager_WorksCorrectly) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    
    const ComponentManager* constManager = Entity::getComponentManager();
    ASSERT_NE(constManager, nullptr);
    
    // Should be able to check if component exists
    EXPECT_TRUE(constManager->hasComponent<CTransform3D>(entity->id()));
    EXPECT_FALSE(constManager->hasComponent<CMovement3D>(entity->id()));
    
    // Should be able to get const component reference
    const CTransform3D& transform = constManager->getComponent<CTransform3D>(entity->id());
    EXPECT_EQ(transform.position, glm::vec3(1.0f, 2.0f, 3.0f));
}

// Test function that takes const Entity reference
void processConstEntity(const Entity& entity) {
    if (entity.has<CTransform3D>()) {
        const auto& transform = entity.get<CTransform3D>();
        // Can read but not modify
        EXPECT_TRUE(transform.exists);
    }
    
    if (entity.hasComponents<CTransform3D, CMovement3D>()) {
        auto [transform, movement] = entity.getComponents<CTransform3D, CMovement3D>();
        // Both should be const references
        static_assert(std::is_const_v<std::remove_reference_t<decltype(transform)>>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(movement)>>);
    }
}

TEST_F(EntityConstCorrectnessTest, ConstEntityFunction_WorksCorrectly) {
    auto entity = createEntity();
    entity->add<CTransform3D>(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    
    // Should be able to pass entity to function expecting const reference
    EXPECT_NO_THROW(processConstEntity(*entity));
}