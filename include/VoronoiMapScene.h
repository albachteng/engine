#pragma once
#include "BaseScene.hpp"
#include "Component.h"
#include "Entity.hpp"
#include "EntityManager.h"
#include "InputController.hpp"
#include "SFMLRenderer.h"
#include "VoronoiGenerator.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

enum class VoronoiMapActions { 
    UP, DOWN, LEFT, RIGHT, SELECT, REGENERATE, 
    TOGGLE_BOUNDARIES, TOGGLE_CENTERS, CYCLE_STYLE, BACK_TO_MENU
};

enum class Direction;  // Forward declaration from MapScene

struct VoronoiMapConfig {
    int regionCount = 15;
    Vec2f mapSize = Vec2f(800, 600);
    unsigned int seed = 42;
    float minRegionDistance = 60.0f;
    int relaxationIterations = 2;
    bool showBoundaries = true;
    bool showCenters = false;
    bool useFantasyColors = true;
};

class VoronoiMapScene : public BaseScene {
private:
    EntityManager m_entityManager;
    std::unique_ptr<SFMLRenderer> m_renderer;
    std::shared_ptr<ActionController<VoronoiMapActions>> m_actionController;
    std::unordered_map<InputEvent, VoronoiMapActions> m_inputMap;
    
    VoronoiMapConfig m_config;
    std::unique_ptr<VoronoiGenerator> m_voronoiGen;
    
    Vec2f m_window_size;
    bool m_paused = false;
    
    // Navigation state
    int m_currentRegionId = 0;
    std::vector<int> m_regionIds;
    
    // Visual debugging
    bool m_showBoundaries = true;
    bool m_showCenters = false;
    bool m_showVoronoiEdges = false;
    
public:
    explicit VoronoiMapScene(sf::RenderWindow& window);
    
    // BaseScene interface
    void onLoad() override;
    void onUnload() override;
    void update(float deltaTime) override;
    void sRender() override;
    void sInput(sf::Event& event, float deltaTime) override;
    void processInput(const InputEvent& event, float deltaTime = 0.0f) override;
    void sMovement(float deltaTime = 0.0f) override;
    
    // Map generation
    void generateVoronoiMap();
    void regenerateWithNewSeed();
    
    // Navigation (public for testing)
    void navigateInDirection(Direction dir);
    int findClosestRegionInDirection(int currentRegionId, Direction dir);
    float calculateRegionDirectionalScore(const Vec2f& from, const Vec2f& to, Direction dir);
    
    // Visual controls
    void toggleBoundaryDisplay();
    void toggleCenterDisplay();
    
    // Utility
    void updateRegionVisuals(float deltaTime);
    Vec2f getRegionCentroid(int regionId);
    
    bool isPaused() { return m_paused; }
    void togglePaused() { m_paused = !m_paused; }
    
    // Public for testing
    VoronoiMapConfig& getConfig() { return m_config; }
    const std::vector<int>& getRegionIds() const { return m_regionIds; }
    int getCurrentRegionId() const { return m_currentRegionId; }
    sf::Color getFantasyColor(int regionId);
    EntityManager& getEntityManager() { return m_entityManager; }
    
private:
    // Helper methods
};