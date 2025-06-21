#include <gtest/gtest.h>
#include "../include/EntityManager.h"
#include "../include/Component.h"

class EntityTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager.clear();
        entity = manager.addEntity("test");
        manager.update(); // Move entity from toAdd to main collection
    }

    EntityManager manager;
    std::shared_ptr<Entity> entity;
};

TEST_F(EntityTest, BasicProperties) {
    EXPECT_EQ(entity->id(), 0); // First entity should have ID 0
    EXPECT_EQ(entity->tag(), "test");
    EXPECT_TRUE(entity->isActive());
}

TEST_F(EntityTest, DestroyEntity) {
    entity->destroy();
    EXPECT_FALSE(entity->isActive());
}

TEST_F(EntityTest, AddComponent_SetsExistsFlag) {
    EXPECT_FALSE(entity->has<CTransform>());
    
    entity->add<CTransform>();
    
    EXPECT_TRUE(entity->has<CTransform>());
}

TEST_F(EntityTest, AddComponent_WithParameters) {
    Vec2f pos(10.0f, 20.0f);
    Vec2f vel(1.0f, 2.0f);
    float angle = 45.0f;
    
    auto& transform = entity->add<CTransform>(pos, vel, angle);
    
    EXPECT_TRUE(entity->has<CTransform>());
    EXPECT_EQ(transform.pos.x, 10.0f);
    EXPECT_EQ(transform.pos.y, 20.0f);
    EXPECT_EQ(transform.vel.x, 1.0f);
    EXPECT_EQ(transform.vel.y, 2.0f);
    EXPECT_EQ(transform.angle, 45.0f);
}

TEST_F(EntityTest, GetComponent_ReturnsReference) {
    entity->add<CScore>(100);
    
    auto& score = entity->get<CScore>();
    score.score = 200;
    
    EXPECT_EQ(entity->get<CScore>().score, 200);
}

TEST_F(EntityTest, RemoveComponent_ClearsExistsFlag) {
    entity->add<CCollision>(5.0f);
    EXPECT_TRUE(entity->has<CCollision>());
    
    entity->remove<CCollision>();
    
    EXPECT_FALSE(entity->has<CCollision>());
}

TEST_F(EntityTest, MultipleComponents) {
    entity->add<CTransform>();
    entity->add<CScore>(50);
    entity->add<CCollision>(3.0f);
    
    EXPECT_TRUE(entity->has<CTransform>());
    EXPECT_TRUE(entity->has<CScore>());
    EXPECT_TRUE(entity->has<CCollision>());
    EXPECT_EQ(entity->get<CScore>().score, 50);
    EXPECT_EQ(entity->get<CCollision>().radius, 3.0f);
}

TEST_F(EntityTest, ComponentLifecycle_KeepsData) {
    entity->add<CLifespan>(100);
    EXPECT_EQ(entity->get<CLifespan>().lifespan, 100);
    EXPECT_EQ(entity->get<CLifespan>().remaining, 100);
    
    entity->get<CLifespan>().remaining = 50;
    EXPECT_EQ(entity->get<CLifespan>().remaining, 50);
}