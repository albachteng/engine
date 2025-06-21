#include <gtest/gtest.h>
#include "../include/Component.h"

class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(ComponentTest, Component_DefaultExistsFalse) {
    CTransform transform;
    EXPECT_FALSE(transform.exists);
}

TEST_F(ComponentTest, CTransform_DefaultValues) {
    CTransform transform;
    EXPECT_EQ(transform.pos.x, 0.0f);
    EXPECT_EQ(transform.pos.y, 0.0f);
    EXPECT_EQ(transform.vel.x, 0.0f);
    EXPECT_EQ(transform.vel.y, 0.0f);
    EXPECT_EQ(transform.angle, 0.0f);
}

TEST_F(ComponentTest, CTransform_ParameterizedConstructor) {
    Vec2f pos(10.0f, 20.0f);
    Vec2f vel(5.0f, -3.0f);
    float angle = 90.0f;
    
    CTransform transform(pos, vel, angle);
    
    EXPECT_EQ(transform.pos.x, 10.0f);
    EXPECT_EQ(transform.pos.y, 20.0f);
    EXPECT_EQ(transform.vel.x, 5.0f);
    EXPECT_EQ(transform.vel.y, -3.0f);
    EXPECT_EQ(transform.angle, 90.0f);
}

TEST_F(ComponentTest, CCollision_DefaultConstructor) {
    CCollision collision;
    EXPECT_EQ(collision.radius, 0.0f);
    EXPECT_FALSE(collision.exists);
}

TEST_F(ComponentTest, CCollision_ParameterizedConstructor) {
    CCollision collision(7.5f);
    EXPECT_EQ(collision.radius, 7.5f);
}

TEST_F(ComponentTest, CScore_DefaultConstructor) {
    CScore score;
    EXPECT_EQ(score.score, 0);
    EXPECT_FALSE(score.exists);
}

TEST_F(ComponentTest, CScore_ParameterizedConstructor) {
    CScore score(1500);
    EXPECT_EQ(score.score, 1500);
}

TEST_F(ComponentTest, CLifespan_DefaultConstructor) {
    CLifespan lifespan;
    EXPECT_EQ(lifespan.lifespan, 0);
    EXPECT_EQ(lifespan.remaining, 0);
    EXPECT_FALSE(lifespan.exists);
}

TEST_F(ComponentTest, CLifespan_ParameterizedConstructor) {
    CLifespan lifespan(120);
    EXPECT_EQ(lifespan.lifespan, 120);
    EXPECT_EQ(lifespan.remaining, 120);
}

TEST_F(ComponentTest, CInput_DefaultState) {
    CInput input;
    EXPECT_FALSE(input.up);
    EXPECT_FALSE(input.down);
    EXPECT_FALSE(input.left);
    EXPECT_FALSE(input.right);
    EXPECT_FALSE(input.shoot);
    EXPECT_FALSE(input.exists);
}

TEST_F(ComponentTest, CGravity_DefaultValues) {
    CGravity gravity;
    EXPECT_EQ(gravity.gravity.x, 0.0f);
    EXPECT_EQ(gravity.gravity.y, 9.8f);
    EXPECT_FALSE(gravity.exists);
}

TEST_F(ComponentTest, CSelection_DefaultConstructor) {
    CSelection selection;
    EXPECT_EQ(selection.grid_position.x, 0);
    EXPECT_EQ(selection.grid_position.y, 0);
    EXPECT_FALSE(selection.exists);
}

TEST_F(ComponentTest, CSelection_ParameterizedConstructor) {
    Vec2i pos(5, 10);
    CSelection selection(pos);
    EXPECT_EQ(selection.grid_position.x, 5);
    EXPECT_EQ(selection.grid_position.y, 10);
}

TEST_F(ComponentTest, CTransform3D_DefaultValues) {
    CTransform3D transform3d;
    EXPECT_EQ(transform3d.position.x, 0.0f);
    EXPECT_EQ(transform3d.position.y, 0.0f);
    EXPECT_EQ(transform3d.position.z, 0.0f);
    EXPECT_EQ(transform3d.rotation.x, 0.0f);
    EXPECT_EQ(transform3d.rotation.y, 0.0f);
    EXPECT_EQ(transform3d.rotation.z, 0.0f);
    EXPECT_EQ(transform3d.scale.x, 1.0f);
    EXPECT_EQ(transform3d.scale.y, 1.0f);
    EXPECT_EQ(transform3d.scale.z, 1.0f);
    EXPECT_FALSE(transform3d.exists);
}

TEST_F(ComponentTest, CMovement3D_DefaultValues) {
    CMovement3D movement;
    EXPECT_EQ(movement.vel.x, 0.0f);
    EXPECT_EQ(movement.vel.y, 0.0f);
    EXPECT_EQ(movement.vel.z, 0.0f);
    EXPECT_EQ(movement.acc.x, 0.0f);
    EXPECT_EQ(movement.acc.y, 0.0f);
    EXPECT_EQ(movement.acc.z, 0.0f);
    EXPECT_FALSE(movement.exists);
}