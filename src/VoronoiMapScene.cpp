#include "../include/VoronoiMapScene.h"
#include "../include/Constants.hpp"
#include "../include/Logger.hpp"
#include "../include/MapScene.h" // For Direction enum
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <memory>

VoronoiMapScene::VoronoiMapScene(sf::RenderWindow &window) {
  m_window_size = (Vec2f)window.getSize();
  m_renderer = std::make_unique<SFMLRenderer>(window);
  m_actionController = std::make_shared<ActionController<VoronoiMapActions>>();

  // Initialize Voronoi systems
  m_config.mapSize = m_window_size;
  m_voronoiGen = std::make_unique<VoronoiGenerator>(m_window_size);

  LOG_INFO_STREAM("VoronoiMapScene: Initialized with window size "
                  << m_window_size.x << "x" << m_window_size.y);
}

void VoronoiMapScene::onLoad() {
  LOG_INFO("VoronoiMapScene: Initializing scene");

  // Register input mappings
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::W}] =
      VoronoiMapActions::UP;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::A}] =
      VoronoiMapActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::S}] =
      VoronoiMapActions::DOWN;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::D}] =
      VoronoiMapActions::RIGHT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Up}] =
      VoronoiMapActions::UP;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Left}] =
      VoronoiMapActions::LEFT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Down}] =
      VoronoiMapActions::DOWN;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Right}] =
      VoronoiMapActions::RIGHT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Enter}] =
      VoronoiMapActions::SELECT;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::R}] =
      VoronoiMapActions::REGENERATE;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::B}] =
      VoronoiMapActions::TOGGLE_BOUNDARIES;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::C}] =
      VoronoiMapActions::TOGGLE_CENTERS;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::T}] =
      VoronoiMapActions::CYCLE_STYLE;
  m_inputMap[InputEvent{InputType::Keyboard, sf::Keyboard::Escape}] =
      VoronoiMapActions::BACK_TO_MENU;

  // Register input listeners
  m_actionController->registerListener(VoronoiMapActions::UP, [this](float) {
    navigateInDirection(Direction::UP);
  });
  m_actionController->registerListener(VoronoiMapActions::LEFT, [this](float) {
    navigateInDirection(Direction::LEFT);
  });
  m_actionController->registerListener(VoronoiMapActions::DOWN, [this](float) {
    navigateInDirection(Direction::DOWN);
  });
  m_actionController->registerListener(VoronoiMapActions::RIGHT, [this](float) {
    navigateInDirection(Direction::RIGHT);
  });
  m_actionController->registerListener(
      VoronoiMapActions::REGENERATE,
      [this](float) { regenerateWithNewSeed(); });
  m_actionController->registerListener(
      VoronoiMapActions::TOGGLE_BOUNDARIES,
      [this](float) { toggleBoundaryDisplay(); });
  m_actionController->registerListener(
      VoronoiMapActions::TOGGLE_CENTERS,
      [this](float) { toggleCenterDisplay(); });

  // Generate initial map
  generateVoronoiMap();

  LOG_INFO("VoronoiMapScene: Scene loading completed");
}

void VoronoiMapScene::onUnload() {
  LOG_INFO("VoronoiMapScene: Unloading scene");
  m_entityManager.clear();
}

void VoronoiMapScene::update(float deltaTime) { m_entityManager.update(); }

void VoronoiMapScene::sRender() {
  m_renderer->render(m_entityManager.getEntities());
}

void VoronoiMapScene::sMovement(float deltaTime) {
  updateRegionVisuals(deltaTime);
}

void VoronoiMapScene::sInput(sf::Event &event, float deltaTime) {
  switch (event.type) {
  case sf::Event::KeyPressed: {
    processInput(InputEvent{InputType::Keyboard, event.key.code}, deltaTime);
    break;
  }
  default:
    break;
  }
}

void VoronoiMapScene::processInput(const InputEvent &input, float deltaTime) {
  auto action = m_inputMap.find(input);
  if (action != m_inputMap.end()) {
    m_actionController->handleEvent(action->second, deltaTime, 0.0f, 0.0f);
  }
}

void VoronoiMapScene::generateVoronoiMap() {
  LOG_INFO("VoronoiMapScene: Generating new Voronoi map");

  // Clear existing entities
  m_entityManager.clear();
  m_regionIds.clear();

  // Generate Voronoi diagram
  m_voronoiGen->clear();
  m_voronoiGen->generateRandomSites(m_config.regionCount,
                                    m_config.minRegionDistance, m_config.seed);
  m_voronoiGen->computeVoronoiDiagram();
  m_voronoiGen->relaxSites(m_config.relaxationIterations);

  const auto &cells = m_voronoiGen->getCells();
  LOG_INFO_STREAM("VoronoiMapScene: Generated " << cells.size()
                                                << " Voronoi cells");

  // Create entities for each cell
  for (const auto &cell : cells) {
    auto entity =
        m_entityManager.addEntity(EntityTag::MAP_NODE); // Reuse existing tag

    sf::Color regionColor = getFantasyColor(cell.cellId);
    sf::Color borderColor = sf::Color::Black;

    auto regionComp = CVoronoiRegion(cell.cellId, cell.centroid, cell.vertices);
    regionComp.baseColor = regionColor;
    regionComp.selectedColor = sf::Color::Yellow;
    regionComp.borderColor = borderColor;
    regionComp.neighborIds = cell.neighborIds;

    // TODO: distortion
    regionComp.distortedBoundary = cell.vertices;

    entity->add<CVoronoiRegion>(regionComp);

    // Add complex shape for rendering with proper colors
    auto shapeComp =
        CComplexShape(cell.vertices, regionColor, borderColor, 2.0f);
    entity->add<CComplexShape>(shapeComp);

    // Add transform at centroid
    entity->add<CTransform>(cell.centroid, Vec2f(0.0f, 0.0f), 0.0f);

    m_regionIds.push_back(cell.cellId);
  }

  // Process deferred entity additions
  m_entityManager.update();

  // Set initial selection
  if (!m_regionIds.empty()) {
    m_currentRegionId = m_regionIds[0];
    for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
      if (e->has<CVoronoiRegion>()) {
        auto &region = e->get<CVoronoiRegion>();
        region.isSelected = (region.regionId == m_currentRegionId);
        if (region.isSelected) {
          region.pulseTimer = 0.0f;
        }
      }
    }
  }

  LOG_INFO_STREAM("VoronoiMapScene: Created " << m_regionIds.size()
                                              << " region entities");
}

void VoronoiMapScene::regenerateWithNewSeed() {
  m_config.seed = static_cast<unsigned int>(time(nullptr)) + rand();
  generateVoronoiMap();
  LOG_INFO_STREAM("VoronoiMapScene: Regenerated map with new seed "
                  << m_config.seed);
}

sf::Color VoronoiMapScene::getFantasyColor(int regionId) {
  // Fantasy-themed color palette
  static const std::vector<sf::Color> fantasyColors = {
      sf::Color(156, 93, 82),   // Mountain brown
      sf::Color(76, 114, 61),   // Forest green
      sf::Color(205, 183, 88),  // Plains gold
      sf::Color(64, 109, 164),  // Lake blue
      sf::Color(139, 69, 139),  // Mystical purple
      sf::Color(180, 142, 173), // Lavender hills
      sf::Color(150, 111, 51),  // Desert tan
      sf::Color(47, 79, 79),    // Dark slate gray
      sf::Color(85, 107, 47),   // Dark olive green
      sf::Color(128, 128, 0),   // Olive
      sf::Color(255, 140, 0),   // Dark orange
      sf::Color(72, 61, 139),   // Dark slate blue
      sf::Color(47, 79, 47),    // Dark green
      sf::Color(160, 82, 45),   // Saddle brown
      sf::Color(112, 128, 144)  // Slate gray
  };

  return fantasyColors[regionId % fantasyColors.size()];
}

void VoronoiMapScene::navigateInDirection(Direction dir) {
  int nextRegionId = findClosestRegionInDirection(m_currentRegionId, dir);
  if (nextRegionId != -1 && nextRegionId != m_currentRegionId) {
    // Update selection
    for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
      if (e->has<CVoronoiRegion>()) {
        auto &region = e->get<CVoronoiRegion>();
        region.isSelected = (region.regionId == nextRegionId);
        if (region.isSelected) {
          region.pulseTimer = 0.0f;
        }
      }
    }
    m_currentRegionId = nextRegionId;
    LOG_DEBUG_STREAM("VoronoiMapScene: Navigated to region "
                     << nextRegionId << " in direction "
                     << static_cast<int>(dir));
  } else {
    LOG_DEBUG_STREAM("VoronoiMapScene: No valid region found in direction "
                     << static_cast<int>(dir));
  }
}

int VoronoiMapScene::findClosestRegionInDirection(int currentRegionId,
                                                  Direction dir) {
  // Find current region centroid
  Vec2f currentPos;
  bool found = false;
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CVoronoiRegion>()) {
      auto &region = e->get<CVoronoiRegion>();
      if (region.regionId == currentRegionId) {
        currentPos = region.centroid;
        found = true;
        break;
      }
    }
  }

  if (!found)
    return -1;

  int bestRegionId = -1;
  float bestScore = -1.0f;

  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CVoronoiRegion>()) {
      auto &region = e->get<CVoronoiRegion>();
      if (region.regionId == currentRegionId || !region.isNavigable)
        continue;

      float score =
          calculateRegionDirectionalScore(currentPos, region.centroid, dir);
      if (score >= 0 && (bestScore < 0 || score < bestScore)) {
        bestScore = score;
        bestRegionId = region.regionId;
      }
    }
  }

  return bestRegionId;
}

float VoronoiMapScene::calculateRegionDirectionalScore(const Vec2f &from,
                                                       const Vec2f &to,
                                                       Direction dir) {
  Vec2f delta = Vec2f(to.x - from.x, to.y - from.y);
  float distance = sqrt(delta.x * delta.x + delta.y * delta.y);

  if (distance < 0.001f)
    return -1.0f;

  Vec2f normalizedDelta = Vec2f(delta.x / distance, delta.y / distance);

  Vec2f dirVector;
  switch (dir) {
  case Direction::UP:
    dirVector = Vec2f(0.0f, -1.0f);
    break;
  case Direction::DOWN:
    dirVector = Vec2f(0.0f, 1.0f);
    break;
  case Direction::LEFT:
    dirVector = Vec2f(-1.0f, 0.0f);
    break;
  case Direction::RIGHT:
    dirVector = Vec2f(1.0f, 0.0f);
    break;
  }

  float dotProduct =
      normalizedDelta.x * dirVector.x + normalizedDelta.y * dirVector.y;

  if (dotProduct < EngineConstants::UI::MAP_DIRECTIONAL_TOLERANCE) {
    return -1.0f;
  }

  return distance * (2.0f - dotProduct);
}

void VoronoiMapScene::updateRegionVisuals(float deltaTime) {
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CVoronoiRegion>() && e->has<CComplexShape>()) {
      auto &region = e->get<CVoronoiRegion>();
      auto &shape = e->get<CComplexShape>();

      if (region.isSelected) {
        // Update pulse animation
        region.pulseTimer += deltaTime * EngineConstants::UI::MAP_PULSE_SPEED;

        // Calculate pulsing effect - interpolate between base color and
        // selection color
        float pulseValue = (sin(region.pulseTimer) + 1.0f) * 0.5f; // 0.0 to 1.0

        // Create pulsing color by blending base and selected colors
        sf::Color baseColor = region.baseColor;
        sf::Color selectedColor = region.selectedColor;

        // Linear interpolation between colors
        int r = static_cast<int>(baseColor.r +
                                 pulseValue * (selectedColor.r - baseColor.r));
        int g = static_cast<int>(baseColor.g +
                                 pulseValue * (selectedColor.g - baseColor.g));
        int b = static_cast<int>(baseColor.b +
                                 pulseValue * (selectedColor.b - baseColor.b));

        shape.fillColor = sf::Color(r, g, b, 255);
        shape.outlineColor = region.selectedColor;
        shape.outlineThickness = EngineConstants::UI::MAP_SELECTION_OUTLINE;
      } else {
        // Standard appearance - use the region's base color
        shape.fillColor = region.baseColor;
        shape.outlineColor = region.borderColor;
        shape.outlineThickness = 2.0f;
      }
    }
  }
}

Vec2f VoronoiMapScene::getRegionCentroid(int regionId) {
  for (auto &e : m_entityManager.getEntities(EntityTag::MAP_NODE)) {
    if (e->has<CVoronoiRegion>()) {
      auto &region = e->get<CVoronoiRegion>();
      if (region.regionId == regionId) {
        return region.centroid;
      }
    }
  }
  return Vec2f(0.0f, 0.0f);
}

void VoronoiMapScene::toggleBoundaryDisplay() {
  m_showBoundaries = !m_showBoundaries;
  LOG_INFO_STREAM("VoronoiMapScene: Boundary display "
                  << (m_showBoundaries ? "enabled" : "disabled"));
}

void VoronoiMapScene::toggleCenterDisplay() {
  m_showCenters = !m_showCenters;
  LOG_INFO_STREAM("VoronoiMapScene: Center display "
                  << (m_showCenters ? "enabled" : "disabled"));
}
