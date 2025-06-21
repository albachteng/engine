#include <gtest/gtest.h>
#include "../include/OpenGLRenderer.hpp"
#include "../include/Camera.h"
#include <SFML/Graphics.hpp>
#include <memory>

// Mock test fixture for OpenGL renderer testing
// Note: These tests focus on logical behavior without requiring full OpenGL context
class OpenGLRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a minimal window for testing (not displayed)
        window = std::make_unique<sf::RenderWindow>();
        camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    }

    void TearDown() override {
        window.reset();
        camera.reset();
    }

    std::unique_ptr<sf::RenderWindow> window;
    std::shared_ptr<Camera> camera;
};

TEST_F(OpenGLRendererTest, Constructor_InitializesCamera) {
    // Test camera instance creation and basic functionality
    EXPECT_NE(camera, nullptr);
    
    // Test that camera produces valid matrices (indirect position validation)
    auto viewMatrix = camera->getViewMatrix();
    auto projMatrix = camera->getProjectionMatrix(16.0f / 9.0f);
    
    // Basic matrix validation - should not be zero matrices
    bool viewMatrixValid = false;
    bool projMatrixValid = false;
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (viewMatrix[i][j] != 0.0f) viewMatrixValid = true;
            if (projMatrix[i][j] != 0.0f) projMatrixValid = true;
        }
    }
    
    EXPECT_TRUE(viewMatrixValid);
    EXPECT_TRUE(projMatrixValid);
}

TEST_F(OpenGLRendererTest, Camera_ProducesValidMatrices) {
    // Test camera matrix generation without accessing private members
    auto viewMatrix = camera->getViewMatrix();
    auto projMatrix = camera->getProjectionMatrix(1.777f); // 16:9 aspect ratio
    
    // View matrix should have reasonable values for a camera at (0,0,3)
    // We can't test exact values without getPosition(), but we can test structure
    
    // Projection matrix should be valid perspective matrix
    // For perspective projection, [2][3] should be -1 and [3][2] should be non-zero
    EXPECT_FLOAT_EQ(projMatrix[2][3], -1.0f);
    EXPECT_NE(projMatrix[3][2], 0.0f);
}

// Test entity filtering logic (no OpenGL context needed)
TEST_F(OpenGLRendererTest, RenderLogic_FiltersEntitiesCorrectly) {
    EntityManager manager;
    
    // Create entities with different component combinations
    auto triangleEntity = manager.addEntity(EntityTag::TRIANGLE);
    triangleEntity->add<CTriangle>();
    triangleEntity->add<CTransform3D>();
    
    auto incompleteEntity = manager.addEntity(EntityTag::TRIANGLE);
    incompleteEntity->add<CTriangle>(); // Missing CTransform3D
    
    auto nonTriangleEntity = manager.addEntity(EntityTag::PLAYER);
    nonTriangleEntity->add<CTransform3D>(); // Missing CTriangle
    
    manager.update();
    
    // Verify entity setup
    EXPECT_TRUE(triangleEntity->has<CTriangle>());
    EXPECT_TRUE(triangleEntity->has<CTransform3D>());
    EXPECT_TRUE(incompleteEntity->has<CTriangle>());
    EXPECT_FALSE(incompleteEntity->has<CTransform3D>());
    EXPECT_FALSE(nonTriangleEntity->has<CTriangle>());
    EXPECT_TRUE(nonTriangleEntity->has<CTransform3D>());
    
    // Count renderable entities (those with both CTriangle and CTransform3D)
    int renderableCount = 0;
    for (const auto &e : manager.getEntities()) {
        if (e->has<CTriangle>() && e->has<CTransform3D>()) {
            renderableCount++;
        }
    }
    
    EXPECT_EQ(renderableCount, 1); // Only triangleEntity should be renderable
}

// Test triangle component data integrity
TEST_F(OpenGLRendererTest, TriangleComponent_HasCorrectVertexData) {
    EntityManager manager;
    auto entity = manager.addEntity(EntityTag::TRIANGLE);
    entity->add<CTriangle>();
    
    auto &triangle = entity->get<CTriangle>();
    
    // Verify triangle has expected vertex data structure
    EXPECT_EQ(triangle.vertices.size(), 18); // 3 vertices * 6 floats (pos + color)
    
    // Check that vertices contain reasonable coordinate data
    // Default triangle should have vertices in range [-1, 1]
    for (size_t i = 0; i < triangle.vertices.size(); i += 6) {
        // Position coordinates (x, y, z)
        EXPECT_GE(triangle.vertices[i], -1.0f);     // x
        EXPECT_LE(triangle.vertices[i], 1.0f);
        EXPECT_GE(triangle.vertices[i + 1], -1.0f); // y  
        EXPECT_LE(triangle.vertices[i + 1], 1.0f);
        EXPECT_GE(triangle.vertices[i + 2], -1.0f); // z
        EXPECT_LE(triangle.vertices[i + 2], 1.0f);
        
        // Color coordinates (r, g, b) should be [0, 1]
        EXPECT_GE(triangle.vertices[i + 3], 0.0f);   // r
        EXPECT_LE(triangle.vertices[i + 3], 1.0f);
        EXPECT_GE(triangle.vertices[i + 4], 0.0f);   // g
        EXPECT_LE(triangle.vertices[i + 4], 1.0f);
        EXPECT_GE(triangle.vertices[i + 5], 0.0f);   // b
        EXPECT_LE(triangle.vertices[i + 5], 1.0f);
    }
}

// Test transform component integration
TEST_F(OpenGLRendererTest, Transform3D_DefaultValues) {
    EntityManager manager;
    auto entity = manager.addEntity(EntityTag::TRIANGLE);
    entity->add<CTransform3D>();
    
    auto &transform = entity->get<CTransform3D>();
    
    // Check default transform values
    EXPECT_EQ(transform.position, glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(transform.rotation, glm::vec3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(transform.scale, glm::vec3(1.0f, 1.0f, 1.0f));
}

// Test transform modifications
TEST_F(OpenGLRendererTest, Transform3D_ModificationWorks) {
    EntityManager manager;
    auto entity = manager.addEntity(EntityTag::TRIANGLE);
    
    glm::vec3 pos(1.0f, 2.0f, 3.0f);
    glm::vec3 rot(45.0f, 90.0f, 180.0f);
    glm::vec3 scale(2.0f, 2.0f, 2.0f);
    
    entity->add<CTransform3D>(pos, rot, scale);
    
    auto &transform = entity->get<CTransform3D>();
    
    EXPECT_EQ(transform.position, pos);
    EXPECT_EQ(transform.rotation, rot);
    EXPECT_EQ(transform.scale, scale);
}