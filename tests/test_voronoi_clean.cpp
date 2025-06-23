#include <gtest/gtest.h>
#include "../include/VoronoiMapScene.h"
#include "../include/VoronoiGenerator.h"
#include "../include/Component.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include "../include/MapScene.h" // For Direction enum
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class VoronoiCleanTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test window for VoronoiMapScene
        window = std::make_unique<sf::RenderWindow>();
        window->create(sf::VideoMode(800, 600), "Test Window", sf::Style::None);
        window->setVisible(false); // Don't show the test window
        
        voronoiScene = std::make_unique<VoronoiMapScene>(*window);
        voronoiScene->onLoad();
    }
    
    void TearDown() override {
        voronoiScene.reset();
        window.reset();
    }
    
    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<VoronoiMapScene> voronoiScene;
};

// Test Voronoi generator basic functionality
TEST_F(VoronoiCleanTest, VoronoiGenerator_BasicGeneration) {
    VoronoiGenerator generator(Vec2f(800, 600));
    
    generator.generateRandomSites(10, 50.0f, 42);
    generator.computeVoronoiDiagram();
    generator.relaxSites(2);
    
    const auto& cells = generator.getCells();
    
    EXPECT_GT(cells.size(), 0);
    EXPECT_LE(cells.size(), 10); // Should not exceed requested count
    
    // Check that all cells have valid properties
    for (const auto& cell : cells) {
        EXPECT_GE(cell.cellId, 0);
        EXPECT_GT(cell.area, 0.0f);
        EXPECT_GE(cell.vertices.size(), 3); // Minimum for a polygon
        
        // Check centroid is within bounds
        EXPECT_GE(cell.centroid.x, 0.0f);
        EXPECT_LE(cell.centroid.x, 800.0f);
        EXPECT_GE(cell.centroid.y, 0.0f);
        EXPECT_LE(cell.centroid.y, 600.0f);
    }
}

// Test fantasy color palette
TEST_F(VoronoiCleanTest, FantasyColors_ValidPalette) {
    // Test that fantasy colors are properly defined and accessible
    for (int i = 0; i < 15; ++i) {
        sf::Color color = voronoiScene->getFantasyColor(i);
        
        // Colors should not be completely black (default)
        EXPECT_TRUE(color.r > 0 || color.g > 0 || color.b > 0);
        EXPECT_EQ(color.a, 255); // Full alpha
    }
}

// Test VoronoiMapScene generation
TEST_F(VoronoiCleanTest, VoronoiMapScene_MapGeneration) {
    // Scene should have generated some regions
    const auto& regionIds = voronoiScene->getRegionIds();
    EXPECT_GT(regionIds.size(), 0);
    EXPECT_LE(regionIds.size(), 15); // Default config
    
    // Should have entities for the regions
    int nodeCount = 0;
    for (auto &e : voronoiScene->getEntityManager().getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CVoronoiRegion>()) {
            nodeCount++;
        }
    }
    
    EXPECT_EQ(nodeCount, regionIds.size());
}

// Test region component properties
TEST_F(VoronoiCleanTest, VoronoiRegion_ComponentProperties) {
    int checkedRegions = 0;
    
    for (auto &e : voronoiScene->getEntityManager().getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CVoronoiRegion>()) {
            auto &region = e->get<CVoronoiRegion>();
            
            EXPECT_GE(region.regionId, 0);
            EXPECT_GT(region.area, 0.0f);
            EXPECT_GE(region.originalVertices.size(), 3);
            EXPECT_EQ(region.distortedBoundary.size(), region.originalVertices.size()); // Should be same for clean Voronoi
            EXPECT_TRUE(region.isNavigable);
            
            // Check colors are properly set
            EXPECT_TRUE(region.baseColor.r > 0 || region.baseColor.g > 0 || region.baseColor.b > 0);
            
            checkedRegions++;
        }
    }
    
    EXPECT_GT(checkedRegions, 0);
}

// Test that entities have required components
TEST_F(VoronoiCleanTest, VoronoiRegion_RequiredComponents) {
    for (auto &e : voronoiScene->getEntityManager().getEntities(EntityTag::MAP_NODE)) {
        // Each region entity should have these components
        EXPECT_TRUE(e->has<CVoronoiRegion>());
        EXPECT_TRUE(e->has<CComplexShape>());
        EXPECT_TRUE(e->has<CTransform>());
        
        if (e->has<CVoronoiRegion>() && e->has<CComplexShape>()) {
            auto &region = e->get<CVoronoiRegion>();
            auto &shape = e->get<CComplexShape>();
            
            // Shape should have vertices matching the region
            EXPECT_EQ(shape.vertices.size(), region.originalVertices.size());
            EXPECT_EQ(shape.type, CComplexShape::POLYGON);
        }
    }
}

// Test navigation functionality
TEST_F(VoronoiCleanTest, VoronoiNavigation_DirectionalMovement) {
    int initialRegion = voronoiScene->getCurrentRegionId();
    
    // Try navigation in each direction
    voronoiScene->navigateInDirection(Direction::RIGHT);
    voronoiScene->navigateInDirection(Direction::LEFT);
    voronoiScene->navigateInDirection(Direction::UP);
    voronoiScene->navigateInDirection(Direction::DOWN);
    
    // Navigation should not crash (basic functional test)
    EXPECT_GE(voronoiScene->getCurrentRegionId(), 0);
}

// Test configuration
TEST_F(VoronoiCleanTest, VoronoiConfig_DefaultValues) {
    auto& config = voronoiScene->getConfig();
    
    EXPECT_EQ(config.regionCount, 15);
    EXPECT_EQ(config.seed, 42);
    EXPECT_GT(config.minRegionDistance, 0.0f);
    EXPECT_GT(config.relaxationIterations, 0);
    EXPECT_TRUE(config.useFantasyColors);
}

// Test regeneration
TEST_F(VoronoiCleanTest, VoronoiMapScene_Regeneration) {
    auto initialSeed = voronoiScene->getConfig().seed;
    auto initialRegionCount = voronoiScene->getRegionIds().size();
    
    // Regenerate with new seed
    voronoiScene->regenerateWithNewSeed();
    
    auto newSeed = voronoiScene->getConfig().seed;
    auto newRegionCount = voronoiScene->getRegionIds().size();
    
    EXPECT_NE(newSeed, initialSeed); // Should have different seed
    EXPECT_GT(newRegionCount, 0);    // Should still have regions
}

// Test clean boundaries (no distortion)
TEST_F(VoronoiCleanTest, VoronoiRegion_CleanBoundaries) {
    for (auto &e : voronoiScene->getEntityManager().getEntities(EntityTag::MAP_NODE)) {
        if (e->has<CVoronoiRegion>()) {
            auto &region = e->get<CVoronoiRegion>();
            
            // Original and distorted boundaries should be identical (no noise)
            EXPECT_EQ(region.originalVertices.size(), region.distortedBoundary.size());
            
            for (size_t i = 0; i < region.originalVertices.size(); ++i) {
                EXPECT_EQ(region.originalVertices[i].x, region.distortedBoundary[i].x);
                EXPECT_EQ(region.originalVertices[i].y, region.distortedBoundary[i].y);
            }
        }
    }
}