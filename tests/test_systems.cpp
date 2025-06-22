#include <gtest/gtest.h>
#include "../include/CollisionDetectionSystem.hpp"
#include "../include/CollisionResolutionSystem.hpp"
#include "../include/BoundarySystem.hpp"
#include "../include/MovementSystem.hpp"
#include "../include/EntityManager.h"
#include "../include/Constants.hpp"
#include <glm/glm.hpp>

class SystemsTest : public ::testing::Test {
protected:
    void SetUp() override {
        entityManager = std::make_unique<EntityManager>();
        collisionDetectionSystem = std::make_unique<CollisionDetectionSystem>();
        collisionResolutionSystem = std::make_unique<CollisionResolutionSystem>();
        movementSystem = std::make_unique<MovementSystem>();
        
        // Set up boundary system with test bounds
        BoundaryConstraint constraint(glm::vec3(-10.0f), glm::vec3(10.0f), BoundaryAction::BOUNCE, 0.9f);
        boundarySystem = std::make_unique<BoundarySystem>(constraint);
    }

    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<CollisionDetectionSystem> collisionDetectionSystem;
    std::unique_ptr<CollisionResolutionSystem> collisionResolutionSystem;
    std::unique_ptr<BoundarySystem> boundarySystem;
    std::unique_ptr<MovementSystem> movementSystem;
};

// ============================================================================
// MovementSystem Tests
// ============================================================================

TEST_F(SystemsTest, MovementSystem_BasicMovement) {
    // Create entity with movement
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    // Update movement
    float deltaTime = 1.0f;
    movementSystem->updateMovement(*entityManager, deltaTime);

    // Check position updated
    const auto& transform = entity->get<CTransform3D>();
    EXPECT_FLOAT_EQ(transform.position.x, 1.0f);
    EXPECT_FLOAT_EQ(transform.position.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.position.z, 0.0f);
}

TEST_F(SystemsTest, MovementSystem_AccelerationIntegration) {
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(0.0f), glm::vec3(2.0f, 0.0f, 0.0f));

    // Process deferred entity additions
    entityManager->update();

    float deltaTime = 1.0f;
    movementSystem->updateMovement(*entityManager, deltaTime);

    // Check velocity updated from acceleration
    const auto& movement = entity->get<CMovement3D>();
    EXPECT_FLOAT_EQ(movement.vel.x, 2.0f);
    
    // Check position updated from velocity
    const auto& transform = entity->get<CTransform3D>();
    EXPECT_FLOAT_EQ(transform.position.x, 2.0f);
}

TEST_F(SystemsTest, MovementSystem_ImpulseApplication) {
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Apply impulse
    glm::vec3 impulse(2.0f, 3.0f, 1.0f);
    movementSystem->applyImpulse(entity, impulse);

    // Check velocity changed
    const auto& movement = entity->get<CMovement3D>();
    EXPECT_FLOAT_EQ(movement.vel.x, 3.0f);
    EXPECT_FLOAT_EQ(movement.vel.y, 3.0f);
    EXPECT_FLOAT_EQ(movement.vel.z, 1.0f);
}

TEST_F(SystemsTest, MovementSystem_SpeedLimit) {
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    // Set speed limit
    movementSystem->setMaxSpeed(entity, 5.0f);
    movementSystem->updateMovement(*entityManager, 1.0f);

    // Check speed was limited
    const auto& movement = entity->get<CMovement3D>();
    float speed = glm::length(movement.vel);
    EXPECT_FLOAT_EQ(speed, 5.0f);
}

// ============================================================================
// BoundarySystem Tests
// ============================================================================

TEST_F(SystemsTest, BoundarySystem_BounceAction) {
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(15.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    boundarySystem->enforceBoundaries(*entityManager);

    // Check position clamped and velocity reversed
    const auto& transform = entity->get<CTransform3D>();
    const auto& movement = entity->get<CMovement3D>();
    
    EXPECT_FLOAT_EQ(transform.position.x, 10.0f); // Clamped to boundary
    EXPECT_LT(movement.vel.x, 0.0f); // Velocity reversed
}

TEST_F(SystemsTest, BoundarySystem_ClampAction) {
    BoundaryConstraint clampConstraint(glm::vec3(-5.0f), glm::vec3(5.0f), BoundaryAction::CLAMP);
    boundarySystem->setBoundaryConstraint(clampConstraint);

    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    boundarySystem->enforceBoundaries(*entityManager);

    // Check position clamped and velocity stopped
    const auto& transform = entity->get<CTransform3D>();
    const auto& movement = entity->get<CMovement3D>();
    
    EXPECT_FLOAT_EQ(transform.position.x, 5.0f);
    EXPECT_FLOAT_EQ(movement.vel.x, 0.0f);
}

TEST_F(SystemsTest, BoundarySystem_WrapAction) {
    BoundaryConstraint wrapConstraint(glm::vec3(-5.0f), glm::vec3(5.0f), BoundaryAction::WRAP);
    boundarySystem->setBoundaryConstraint(wrapConstraint);

    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    boundarySystem->enforceBoundaries(*entityManager);

    // Check position wrapped to opposite side
    const auto& transform = entity->get<CTransform3D>();
    EXPECT_FLOAT_EQ(transform.position.x, -5.0f);
}

// ============================================================================
// CollisionDetectionSystem Tests
// ============================================================================

TEST_F(SystemsTest, CollisionDetectionSystem_AABBCollision) {
    // Create two overlapping AABBs
    CAABB aabb1;
    aabb1.min = glm::vec3(0.0f, 0.0f, 0.0f);
    aabb1.max = glm::vec3(2.0f, 2.0f, 2.0f);

    CAABB aabb2;
    aabb2.min = glm::vec3(1.0f, 1.0f, 1.0f);
    aabb2.max = glm::vec3(3.0f, 3.0f, 3.0f);

    bool collision = collisionDetectionSystem->checkAABBCollision(aabb1, aabb2);
    EXPECT_TRUE(collision);
}

TEST_F(SystemsTest, CollisionDetectionSystem_NoCollision) {
    CAABB aabb1;
    aabb1.min = glm::vec3(0.0f, 0.0f, 0.0f);
    aabb1.max = glm::vec3(1.0f, 1.0f, 1.0f);

    CAABB aabb2;
    aabb2.min = glm::vec3(2.0f, 2.0f, 2.0f);
    aabb2.max = glm::vec3(3.0f, 3.0f, 3.0f);

    bool collision = collisionDetectionSystem->checkAABBCollision(aabb1, aabb2);
    EXPECT_FALSE(collision);
}

TEST_F(SystemsTest, CollisionDetectionSystem_EntityCollisionDetection) {
    // Create two entities with overlapping AABBs
    auto entity1 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity1->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity1->add<CAABB>(glm::vec3(0.0f), glm::vec3(1.0f));

    auto entity2 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity2->add<CTransform3D>(glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity2->add<CAABB>(glm::vec3(0.0f), glm::vec3(1.0f));

    // Process deferred entity additions
    entityManager->update();

    auto collisions = collisionDetectionSystem->detectCollisions(*entityManager);
    EXPECT_EQ(collisions.size(), 1);
    EXPECT_EQ(collisions[0].entityA->id(), entity1->id());
    EXPECT_EQ(collisions[0].entityB->id(), entity2->id());
}

// ============================================================================
// CollisionResolutionSystem Tests
// ============================================================================

TEST_F(SystemsTest, CollisionResolutionSystem_ElasticCollision) {
    collisionResolutionSystem->setDefaultResponse(CollisionResponseType::ELASTIC, 1.0f);

    auto entity1 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity1->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity1->add<CMovement3D>(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    auto entity2 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity2->add<CTransform3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity2->add<CMovement3D>(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    CollisionEvent collision(entity1, entity2);
    collision.contactNormal = glm::vec3(1.0f, 0.0f, 0.0f);
    collision.penetrationDepth = 0.1f;

    collisionResolutionSystem->resolveCollision(collision);

    // After elastic collision, velocities should be exchanged
    const auto& movement1 = entity1->get<CMovement3D>();
    const auto& movement2 = entity2->get<CMovement3D>();

    EXPECT_LT(movement1.vel.x, 0.0f); // Entity1 now moving left
    EXPECT_GT(movement2.vel.x, 0.0f); // Entity2 now moving right
}

TEST_F(SystemsTest, CollisionResolutionSystem_AbsorbCollision) {
    collisionResolutionSystem->setDefaultResponse(CollisionResponseType::ABSORB);

    auto entity1 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity1->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity1->add<CMovement3D>(glm::vec3(5.0f, 3.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    auto entity2 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity2->add<CTransform3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity2->add<CMovement3D>(glm::vec3(-2.0f, 1.0f, -1.0f), glm::vec3(-0.5f, 0.5f, 0.2f));

    // Process deferred entity additions
    entityManager->update();

    CollisionEvent collision(entity1, entity2);
    collisionResolutionSystem->resolveCollision(collision);

    // After absorb collision, both entities should stop
    const auto& movement1 = entity1->get<CMovement3D>();
    const auto& movement2 = entity2->get<CMovement3D>();

    EXPECT_EQ(movement1.vel, glm::vec3(0.0f));
    EXPECT_EQ(movement2.vel, glm::vec3(0.0f));
    EXPECT_EQ(movement1.acc, glm::vec3(0.0f));
    EXPECT_EQ(movement2.acc, glm::vec3(0.0f));
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(SystemsTest, Integration_MovementAndBoundarySystem) {
    // Create entity moving toward boundary
    auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>(glm::vec3(8.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity->add<CMovement3D>(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f));

    // Process deferred entity additions
    entityManager->update();

    // Update movement first
    movementSystem->updateMovement(*entityManager, 1.0f);
    
    // Then apply boundary constraints
    boundarySystem->enforceBoundaries(*entityManager);

    // Entity should be at boundary with reversed velocity
    const auto& transform = entity->get<CTransform3D>();
    const auto& movement = entity->get<CMovement3D>();
    
    EXPECT_FLOAT_EQ(transform.position.x, 10.0f);
    EXPECT_LT(movement.vel.x, 0.0f);
}

TEST_F(SystemsTest, Integration_FullPhysicsLoop) {
    // Create two entities that will collide
    auto entity1 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity1->add<CTransform3D>(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity1->add<CMovement3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    entity1->add<CAABB>(glm::vec3(0.0f), glm::vec3(0.5f));

    auto entity2 = entityManager->addEntity(EntityTag::TRIANGLE);
    entity2->add<CTransform3D>(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    entity2->add<CMovement3D>(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f));
    entity2->add<CAABB>(glm::vec3(0.0f), glm::vec3(0.5f));

    // Process deferred entity additions
    entityManager->update();

    // Run physics simulation for several steps
    for (int i = 0; i < 10; ++i) {
        movementSystem->updateMovement(*entityManager, 0.1f);
        auto collisions = collisionDetectionSystem->detectCollisions(*entityManager);
        collisionResolutionSystem->resolveCollisions(collisions);
        boundarySystem->enforceBoundaries(*entityManager);
    }

    // Entities should have interacted (either collided or bounced off boundaries)
    const auto& movement1 = entity1->get<CMovement3D>();
    const auto& movement2 = entity2->get<CMovement3D>();
    
    // At least one entity should have changed direction
    bool entity1Changed = movement1.vel.x < 0.0f; // Originally moving right
    bool entity2Changed = movement2.vel.x > 0.0f; // Originally moving left
    
    EXPECT_TRUE(entity1Changed || entity2Changed);
}