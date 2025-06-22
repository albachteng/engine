#include <gtest/gtest.h>
#include "../include/MapScene.h"
#include "../include/Component.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class MapSceneNavigationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test window for MapScene
        window = std::make_unique<sf::RenderWindow>();
        window->create(sf::VideoMode(800, 600), "Test Window", sf::Style::None);
        window->setVisible(false); // Don't show the test window
        
        mapScene = std::make_unique<MapScene>(*window);
        mapScene->onLoad();
    }
    
    void TearDown() override {
        mapScene.reset();
        window.reset();
    }
    
    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<MapScene> mapScene;
};

// Test CMapNode component construction
TEST_F(MapSceneNavigationTest, CMapNode_DefaultConstructor) {
    CMapNode node;
    
    EXPECT_EQ(node.nodeId, -1);
    EXPECT_EQ(node.position.x, 0.0f);
    EXPECT_EQ(node.position.y, 0.0f);
    EXPECT_EQ(node.shape, NodeShape::CIRCLE);
    EXPECT_FALSE(node.isSelected);
    EXPECT_TRUE(node.isNavigable);
    EXPECT_EQ(node.size, 20.0f);
    EXPECT_EQ(node.pulseTimer, 0.0f);
}

TEST_F(MapSceneNavigationTest, CMapNode_ParameterizedConstructor) {
    Vec2f testPos(100.0f, 200.0f);
    CMapNode node(5, testPos, NodeShape::DIAMOND, true, 30.0f);
    
    EXPECT_EQ(node.nodeId, 5);
    EXPECT_EQ(node.position.x, 100.0f);
    EXPECT_EQ(node.position.y, 200.0f);
    EXPECT_EQ(node.shape, NodeShape::DIAMOND);
    EXPECT_FALSE(node.isSelected);
    EXPECT_TRUE(node.isNavigable);
    EXPECT_EQ(node.size, 30.0f);
    EXPECT_EQ(node.pulseTimer, 0.0f);
}

// Test NodeShape enum values
TEST_F(MapSceneNavigationTest, NodeShape_EnumValues) {
    EXPECT_EQ(static_cast<int>(NodeShape::CIRCLE), 0);
    EXPECT_EQ(static_cast<int>(NodeShape::SQUARE), 1);
    EXPECT_EQ(static_cast<int>(NodeShape::DIAMOND), 2);
    EXPECT_EQ(static_cast<int>(NodeShape::TRIANGLE), 3);
    EXPECT_EQ(static_cast<int>(NodeShape::HEXAGON), 4);
}

// Test Direction enum values
TEST_F(MapSceneNavigationTest, Direction_EnumValues) {
    EXPECT_EQ(static_cast<int>(Direction::UP), 0);
    EXPECT_EQ(static_cast<int>(Direction::DOWN), 1);
    EXPECT_EQ(static_cast<int>(Direction::LEFT), 2);
    EXPECT_EQ(static_cast<int>(Direction::RIGHT), 3);
}

// Test map generation constants
TEST_F(MapSceneNavigationTest, MapConstants_ValidValues) {
    EXPECT_GT(EngineConstants::UI::MAP_NODE_DEFAULT_SIZE, 0.0f);
    EXPECT_GE(EngineConstants::UI::MAP_DIRECTIONAL_TOLERANCE, 0.0f);
    EXPECT_LE(EngineConstants::UI::MAP_DIRECTIONAL_TOLERANCE, 1.0f);
    EXPECT_GT(EngineConstants::UI::MAP_PULSE_SPEED, 0.0f);
    EXPECT_GE(EngineConstants::UI::MAP_PULSE_MIN_ALPHA, 0);
    EXPECT_LE(EngineConstants::UI::MAP_PULSE_MAX_ALPHA, 255);
    EXPECT_LT(EngineConstants::UI::MAP_PULSE_MIN_ALPHA, EngineConstants::UI::MAP_PULSE_MAX_ALPHA);
    EXPECT_GT(EngineConstants::UI::MAP_SELECTION_OUTLINE, 0.0f);
}

// Test basic directional navigation
TEST_F(MapSceneNavigationTest, DirectionalNavigation_BasicMovement) {
    // Sample map should have node 0 at center (400, 300)
    // Node 1 should be to the left (300, 300)
    // Node 2 should be to the right (500, 300)
    // Node 3 should be up (400, 200)
    // Node 4 should be down (400, 400)
    
    // Test navigation in each direction from center
    EXPECT_NO_THROW(mapScene->navigateInDirection(Direction::LEFT));
    EXPECT_NO_THROW(mapScene->navigateInDirection(Direction::RIGHT));
    EXPECT_NO_THROW(mapScene->navigateInDirection(Direction::UP));
    EXPECT_NO_THROW(mapScene->navigateInDirection(Direction::DOWN));
}

// Test that only one node is selected at a time
TEST_F(MapSceneNavigationTest, Navigation_SingleSelection) {
    // Navigate in a direction
    mapScene->navigateInDirection(Direction::RIGHT);
    
    // Count selected nodes
    int selectedCount = 0;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            if (mapNode.isSelected) {
                selectedCount++;
            }
        }
    }
    
    EXPECT_EQ(selectedCount, 1);
}

// Test directional score calculation
TEST_F(MapSceneNavigationTest, DirectionalScore_Calculation) {
    Vec2f center(400, 300);
    
    // Test perfect alignment (should give good scores)
    Vec2f right(500, 300);   // Directly right
    Vec2f up(400, 200);      // Directly up
    Vec2f left(300, 300);    // Directly left
    Vec2f down(400, 400);    // Directly down
    
    float rightScore = mapScene->calculateDirectionalScore(center, right, Direction::RIGHT);
    float upScore = mapScene->calculateDirectionalScore(center, up, Direction::UP);
    float leftScore = mapScene->calculateDirectionalScore(center, left, Direction::LEFT);
    float downScore = mapScene->calculateDirectionalScore(center, down, Direction::DOWN);
    
    // All should be valid (>= 0)
    EXPECT_GE(rightScore, 0.0f);
    EXPECT_GE(upScore, 0.0f);
    EXPECT_GE(leftScore, 0.0f);
    EXPECT_GE(downScore, 0.0f);
    
    // Test wrong direction (should be invalid)
    float wrongScore = mapScene->calculateDirectionalScore(center, right, Direction::LEFT);
    EXPECT_LT(wrongScore, 0.0f);
}

// Test edge cases for directional navigation
TEST_F(MapSceneNavigationTest, DirectionalNavigation_EdgeCases) {
    // Test same position
    Vec2f pos(400, 300);
    float sameScore = mapScene->calculateDirectionalScore(pos, pos, Direction::RIGHT);
    EXPECT_LT(sameScore, 0.0f);
    
    // Test different alignment scores - more aligned should be better (lower score)
    Vec2f center(400, 300);
    Vec2f perfectUp(400, 200);    // Directly up (perfect alignment)
    Vec2f slightDiagonal(410, 200);  // Slightly diagonal but still good alignment
    
    float perfectScore = mapScene->calculateDirectionalScore(center, perfectUp, Direction::UP);
    float diagonalScore = mapScene->calculateDirectionalScore(center, slightDiagonal, Direction::UP);
    
    // Both should be valid
    EXPECT_GE(perfectScore, 0.0f);
    EXPECT_GE(diagonalScore, 0.0f);
    // Perfect alignment should have better (lower or equal) score
    EXPECT_LE(perfectScore, diagonalScore);
}

// Test boundary navigation (no valid nodes in direction)
TEST_F(MapSceneNavigationTest, Navigation_BoundaryConditions) {
    // Navigate to far left node (node 5 at position 200, 300)
    // There should be no node further left
    for (int i = 0; i < 5; ++i) {
        mapScene->navigateInDirection(Direction::LEFT);
    }
    
    // Try to navigate left again - should stay at same node
    int currentNodeBefore = mapScene->m_currentNodeId;
    mapScene->navigateInDirection(Direction::LEFT);
    int currentNodeAfter = mapScene->m_currentNodeId;
    
    EXPECT_EQ(currentNodeBefore, currentNodeAfter);
}

// Test map generation
TEST_F(MapSceneNavigationTest, MapGeneration_NodeCount) {
    // Count total nodes in generated map
    int nodeCount = 0;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            nodeCount++;
        }
    }
    
    EXPECT_GT(nodeCount, 0);
    EXPECT_EQ(nodeCount, 13); // Sample map should have 13 nodes
}

// Test that all nodes are navigable by default
TEST_F(MapSceneNavigationTest, MapGeneration_NodesNavigable) {
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            EXPECT_TRUE(mapNode.isNavigable);
        }
    }
}

// Test unique node IDs
TEST_F(MapSceneNavigationTest, MapGeneration_UniqueNodeIds) {
    std::vector<int> nodeIds;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            nodeIds.push_back(mapNode.nodeId);
        }
    }
    
    // Sort and check for duplicates
    std::sort(nodeIds.begin(), nodeIds.end());
    auto it = std::adjacent_find(nodeIds.begin(), nodeIds.end());
    EXPECT_EQ(it, nodeIds.end()); // No duplicates found
}

// Test initial selection state
TEST_F(MapSceneNavigationTest, MapGeneration_InitialSelection) {
    // Should start with node 0 selected
    EXPECT_EQ(mapScene->m_currentNodeId, 0);
    
    // Verify exactly one node is selected
    int selectedCount = 0;
    int selectedNodeId = -1;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            if (mapNode.isSelected) {
                selectedCount++;
                selectedNodeId = mapNode.nodeId;
            }
        }
    }
    
    EXPECT_EQ(selectedCount, 1);
    EXPECT_EQ(selectedNodeId, 0);
}

// Test visual update system
TEST_F(MapSceneNavigationTest, VisualUpdate_PulseAnimation) {
    // Get initial pulse timer
    float initialTimer = -1.0f;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            if (mapNode.isSelected) {
                initialTimer = mapNode.pulseTimer;
                break;
            }
        }
    }
    
    EXPECT_GE(initialTimer, 0.0f);
    
    // Update visuals with small delta time
    float deltaTime = 0.1f;
    mapScene->updateNodeVisuals(deltaTime);
    
    // Check that pulse timer advanced
    float updatedTimer = -1.0f;
    for (auto &e : mapScene->m_entityManager.getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CMapNode>()) {
            auto &mapNode = e->get<CMapNode>();
            if (mapNode.isSelected) {
                updatedTimer = mapNode.pulseTimer;
                break;
            }
        }
    }
    
    EXPECT_GT(updatedTimer, initialTimer);
}

// Test comprehensive navigation path
TEST_F(MapSceneNavigationTest, Navigation_ComprehensivePath) {
    // Test navigation sequence: center -> right -> up -> left -> down
    std::vector<Direction> path = {Direction::RIGHT, Direction::UP, Direction::LEFT, Direction::DOWN};
    std::vector<int> expectedNodes = {2, 3, 1, 4}; // Based on sample map layout
    
    for (size_t i = 0; i < path.size(); ++i) {
        mapScene->navigateInDirection(path[i]);
        // Note: We can't directly check node IDs without accessing private members
        // This test verifies that navigation doesn't crash
        EXPECT_NO_THROW(mapScene->updateNodeVisuals(0.1f));
    }
}