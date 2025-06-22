#include <gtest/gtest.h>
#include "../include/GameScene.h"
#include "../include/Component.h"
#include "../include/Constants.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class GridCameraTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a minimal window for testing
        window = std::make_unique<sf::RenderWindow>();
        window->create(sf::VideoMode(800, 600), "Test Window");
        
        // Create GameScene for testing
        gameScene = std::make_unique<GameScene>(*window);
    }

    void TearDown() override {
        gameScene.reset();
        if (window && window->isOpen()) {
            window->close();
        }
        window.reset();
    }

    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<GameScene> gameScene;
};

// ============================================================================
// Grid System Tests
// ============================================================================

TEST_F(GridCameraTest, GridComponent_DefaultConstructor) {
    CGridLine gridLine;
    EXPECT_EQ(gridLine.start, glm::vec3(0.0f));
    EXPECT_EQ(gridLine.end, glm::vec3(0.0f));
    EXPECT_EQ(gridLine.color, glm::vec3(0.0f));
    EXPECT_EQ(gridLine.width, 0.0f);
    EXPECT_FALSE(gridLine.isMajor);
}

TEST_F(GridCameraTest, GridComponent_ParameterizedConstructor) {
    glm::vec3 start(1.0f, 2.0f, 3.0f);
    glm::vec3 end(4.0f, 5.0f, 6.0f);
    glm::vec3 color(0.5f, 0.7f, 0.9f);
    float width = 0.1f;
    bool isMajor = true;
    
    CGridLine gridLine(start, end, color, width, isMajor);
    
    EXPECT_EQ(gridLine.start, start);
    EXPECT_EQ(gridLine.end, end);
    EXPECT_EQ(gridLine.color, color);
    EXPECT_EQ(gridLine.width, width);
    EXPECT_TRUE(gridLine.isMajor);
}

TEST_F(GridCameraTest, GridComponent_DefaultColor) {
    glm::vec3 start(0.0f, 0.0f, 0.0f);
    glm::vec3 end(10.0f, 0.0f, 0.0f);
    
    CGridLine gridLine(start, end);
    
    EXPECT_EQ(gridLine.color, glm::vec3(0.5f, 0.5f, 0.5f));
    EXPECT_EQ(gridLine.width, 0.02f);
    EXPECT_FALSE(gridLine.isMajor);
}

TEST_F(GridCameraTest, GridToggle_InitialState) {
    // Grid should be visible by default according to constants
    EXPECT_TRUE(EngineConstants::UI::GRID_3D_DEFAULT_VISIBLE);
}

TEST_F(GridCameraTest, GridConstants_ValidValues) {
    // Test that grid constants are reasonable
    EXPECT_GT(EngineConstants::UI::GRID_3D_SIZE, 0.0f);
    EXPECT_GT(EngineConstants::UI::GRID_3D_SPACING, 0.0f);
    EXPECT_GT(EngineConstants::UI::GRID_3D_LINE_WIDTH, 0.0f);
    EXPECT_GT(EngineConstants::UI::GRID_3D_MAJOR_SPACING, EngineConstants::UI::GRID_3D_SPACING);
}

// ============================================================================
// Mouse Handling Tests
// ============================================================================

TEST_F(GridCameraTest, MouseCapture_Constants) {
    // Test that mouse capture constants are defined
    EXPECT_TRUE(EngineConstants::Input::ENABLE_MOUSE_CAPTURE);
    EXPECT_GT(EngineConstants::Input::MOUSE_MOVEMENT_THRESHOLD, 0);
}

TEST_F(GridCameraTest, SceneActions_EnumValues) {
    // Test that new scene actions are defined
    SceneActions mouseToggle = SceneActions::MOUSE_TOGGLE;
    SceneActions gridToggle = SceneActions::GRID_TOGGLE;
    
    // Just verify they compile and can be compared
    EXPECT_NE(mouseToggle, gridToggle);
    EXPECT_NE(mouseToggle, SceneActions::FORWARD);
    EXPECT_NE(gridToggle, SceneActions::BACK);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(GridCameraTest, GameScene_Initialization) {
    // Test that GameScene initializes without crashing
    EXPECT_NO_THROW({
        gameScene->onLoad();
    });
}

TEST_F(GridCameraTest, GridLineEntity_CanBeCreated) {
    // Test that we can create entities with grid line components
    auto entityManager = std::make_unique<EntityManager>();
    
    auto gridEntity = entityManager->addEntity(EntityTag::TRIANGLE);
    gridEntity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    gridEntity->add<CGridLine>(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(5.0f, 0.0f, 0.0f));
    
    entityManager->update();
    
    EXPECT_TRUE(gridEntity->has<CGridLine>());
    EXPECT_TRUE(gridEntity->has<CTransform3D>());
    
    const auto& gridLine = gridEntity->get<CGridLine>();
    EXPECT_EQ(gridLine.start, glm::vec3(-5.0f, 0.0f, 0.0f));
    EXPECT_EQ(gridLine.end, glm::vec3(5.0f, 0.0f, 0.0f));
}

TEST_F(GridCameraTest, MultipleGridLines_CanBeCreated) {
    auto entityManager = std::make_unique<EntityManager>();
    
    // Create multiple grid lines
    std::vector<std::shared_ptr<Entity>> gridEntities;
    
    for (int i = 0; i < 5; ++i) {
        auto entity = entityManager->addEntity(EntityTag::TRIANGLE);
        entity->add<CTransform3D>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
        entity->add<CGridLine>(glm::vec3(i, 0, -10), glm::vec3(i, 0, 10));
        gridEntities.push_back(entity);
    }
    
    entityManager->update();
    
    // Verify all entities have the required components
    for (const auto& entity : gridEntities) {
        EXPECT_TRUE(entity->has<CGridLine>());
        EXPECT_TRUE(entity->has<CTransform3D>());
    }
    
    // Verify we can distinguish between different grid lines
    EXPECT_NE(gridEntities[0]->get<CGridLine>().start.x, 
              gridEntities[1]->get<CGridLine>().start.x);
}

TEST_F(GridCameraTest, GridLine_ColorVariations) {
    // Test major vs minor grid line colors
    CGridLine minorLine(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), 
                        glm::vec3(0.4f, 0.4f, 0.4f), 0.02f, false);
    CGridLine majorLine(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), 
                        glm::vec3(0.8f, 0.8f, 0.8f), 0.02f, true);
    
    EXPECT_FALSE(minorLine.isMajor);
    EXPECT_TRUE(majorLine.isMajor);
    
    // Major lines should be brighter (higher color values)
    EXPECT_GT(majorLine.color.r, minorLine.color.r);
    EXPECT_GT(majorLine.color.g, minorLine.color.g);
    EXPECT_GT(majorLine.color.b, minorLine.color.b);
}

TEST_F(GridCameraTest, CoordinateAxes_Colors) {
    // Test standard coordinate axis colors (X=red, Y=green, Z=blue)
    CGridLine xAxis(glm::vec3(-10, 0, 0), glm::vec3(10, 0, 0), 
                    glm::vec3(1.0f, 0.0f, 0.0f), 0.05f, true);
    CGridLine yAxis(glm::vec3(0, -10, 0), glm::vec3(0, 10, 0), 
                    glm::vec3(0.0f, 1.0f, 0.0f), 0.05f, true);
    CGridLine zAxis(glm::vec3(0, 0, -10), glm::vec3(0, 0, 10), 
                    glm::vec3(0.0f, 0.0f, 1.0f), 0.05f, true);
    
    EXPECT_EQ(xAxis.color, glm::vec3(1.0f, 0.0f, 0.0f)); // Red
    EXPECT_EQ(yAxis.color, glm::vec3(0.0f, 1.0f, 0.0f)); // Green
    EXPECT_EQ(zAxis.color, glm::vec3(0.0f, 0.0f, 1.0f)); // Blue
    
    EXPECT_TRUE(xAxis.isMajor);
    EXPECT_TRUE(yAxis.isMajor);
    EXPECT_TRUE(zAxis.isMajor);
}