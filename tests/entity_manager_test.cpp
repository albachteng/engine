#include <gtest/gtest.h>
#include "../include/EntityManager.h"
#include "../include/Component.h"

class EntityManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager.clear(); // Ensure clean state
    }
    
    EntityManager manager;
};

TEST_F(EntityManagerTest, AddEntity_CreatesEntityWithCorrectTag) {
    auto entity = manager.addEntity("player");
    
    EXPECT_EQ(entity->tag(), "player");
    EXPECT_TRUE(entity->isActive());
}

TEST_F(EntityManagerTest, AddEntity_AssignsUniqueIDs) {
    auto entity1 = manager.addEntity("player");
    auto entity2 = manager.addEntity("enemy");
    
    EXPECT_NE(entity1->id(), entity2->id());
}

TEST_F(EntityManagerTest, Update_MovesEntitiesToMainCollection) {
    auto entity = manager.addEntity("test");
    EXPECT_EQ(manager.getEntities().size(), 0); // Not moved yet
    
    manager.update();
    
    EXPECT_EQ(manager.getEntities().size(), 1);
    EXPECT_EQ(manager.getEntities()[0], entity);
}

TEST_F(EntityManagerTest, Update_MovesEntitiesToTaggedCollection) {
    auto player = manager.addEntity("player");
    auto enemy = manager.addEntity("enemy");
    
    manager.update();
    
    EXPECT_EQ(manager.getEntities("player").size(), 1);
    EXPECT_EQ(manager.getEntities("enemy").size(), 1);
    EXPECT_EQ(manager.getEntities("player")[0], player);
    EXPECT_EQ(manager.getEntities("enemy")[0], enemy);
}

TEST_F(EntityManagerTest, Update_RemovesInactiveFromMainCollection) {
    auto entity1 = manager.addEntity("test");
    auto entity2 = manager.addEntity("test");
    manager.update();
    
    entity1->destroy();
    manager.update();
    
    EXPECT_EQ(manager.getEntities().size(), 1);
    EXPECT_EQ(manager.getEntities()[0], entity2);
}

TEST_F(EntityManagerTest, Update_RemovesInactiveFromTaggedCollections) {
    auto player1 = manager.addEntity("player");
    auto player2 = manager.addEntity("player");
    auto enemy = manager.addEntity("enemy");
    manager.update();
    
    player1->destroy();
    manager.update();
    
    EXPECT_EQ(manager.getEntities("player").size(), 1);
    EXPECT_EQ(manager.getEntities("enemy").size(), 1);
    EXPECT_EQ(manager.getEntities("player")[0], player2);
    EXPECT_EQ(manager.getEntities("enemy")[0], enemy);
}

TEST_F(EntityManagerTest, Clear_RemovesAllEntities) {
    manager.addEntity("player");
    manager.addEntity("enemy");
    manager.update();
    
    manager.clear();
    
    EXPECT_EQ(manager.getEntities().size(), 0);
    EXPECT_EQ(manager.getEntities("player").size(), 0);
    EXPECT_EQ(manager.getEntities("enemy").size(), 0);
}

TEST_F(EntityManagerTest, Clear_ResetsTotalEntityCount) {
    auto entity1 = manager.addEntity("test");
    manager.clear();
    auto entity2 = manager.addEntity("test");
    
    // After clear, new entities should start from ID 0 again
    EXPECT_EQ(entity2->id(), 0);
}

TEST_F(EntityManagerTest, GetEntitiesByTag_ReturnsEmpty_ForNonexistentTag) {
    manager.addEntity("player");
    manager.update();
    
    EXPECT_EQ(manager.getEntities("nonexistent").size(), 0);
}

TEST_F(EntityManagerTest, ComplexLifecycle_MultipleUpdates) {
    // Add entities
    auto p1 = manager.addEntity("player");
    auto e1 = manager.addEntity("enemy");
    manager.update();
    
    // Add more entities
    auto p2 = manager.addEntity("player");
    auto e2 = manager.addEntity("enemy");
    manager.update();
    
    // Destroy some entities
    p1->destroy();
    e2->destroy();
    manager.update();
    
    // Verify final state
    EXPECT_EQ(manager.getEntities().size(), 2);
    EXPECT_EQ(manager.getEntities("player").size(), 1);
    EXPECT_EQ(manager.getEntities("enemy").size(), 1);
    EXPECT_EQ(manager.getEntities("player")[0], p2);
    EXPECT_EQ(manager.getEntities("enemy")[0], e1);
}