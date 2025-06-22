#pragma once
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <glm/ext/vector_float3.hpp>

typedef sf::Vector2f Vec2f;
typedef sf::Vector2i Vec2i;

class Component {
public:
  bool exists = false;
};

class CTransform : public Component {
public:
  Vec2f pos = {0.0, 0.0};
  Vec2f vel = {0.0, 0.0};
  float angle = 0;
  CTransform();
  CTransform(const Vec2f &p, const Vec2f &v, float a);
};

class CTransform3D : public Component {
public:
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};
  CTransform3D();
  CTransform3D(const glm::vec3 &position, const glm::vec3 &rotation,
               const glm::vec3 &scale);
};

class CShape : public Component {
public:
  sf::CircleShape circle;
  CShape();
  CShape(float radius, size_t points, const sf::Color &fill,
         const sf::Color &outline, float thickness);
};

class CCollision : public Component {
private:
public:
  float radius = 0;
  CCollision();
  CCollision(float r);
};

class CScore : public Component {
public:
  int score = 0;
  CScore();
  CScore(int s);
};

class CLifespan : public Component {
public:
  int lifespan = 0;
  int remaining = 0;
  CLifespan();
  CLifespan(int totalLifespan);
};

class CInput : public Component {
public:
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool shoot = false;
  CInput();
};

class CBoundingBox : public Component {
public:
  Vec2f size;
  Vec2f halfsize;
  CBoundingBox(const Vec2f &s);
};

class CGravity : public Component {
public:
  Vec2f gravity = {EngineConstants::Physics::GRAVITY_X, EngineConstants::Physics::GRAVITY_Y};
  CGravity();
};

class CTriangle : public Component {
  // A simple triangle in 3D
  // 18 floats: 3 vertices, each with x, y, z, R, G, B float values
public:
  std::vector<float> vertices = {
      // vertices		  // colors
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom right
      0.0f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f  // top center
  };
  CTriangle();
  // Optionally, add indices, colors, etc.
};

class CAABB : public Component {
public:
  glm::vec3 min;
  glm::vec3 max;

  CAABB();
  CAABB(const glm::vec3 &center, const glm::vec3 &halfsize);
};

class CMovement3D : public Component {
public:
  glm::vec3 vel;
  glm::vec3 acc;

  CMovement3D();
  CMovement3D(const glm::vec3 &velocity, const glm::vec3 &acceleration);
};

class CSelection : public Component {
public:
  Vec2i grid_position;

  CSelection();
  CSelection(Vec2i pos);
};

class CGridLine : public Component {
public:
  glm::vec3 start;
  glm::vec3 end;
  glm::vec3 color;
  float width;
  bool isMajor; // true for major grid lines (e.g., every 10 units)
  
  CGridLine();
  CGridLine(const glm::vec3& startPos, const glm::vec3& endPos, 
            const glm::vec3& lineColor = glm::vec3(0.5f, 0.5f, 0.5f), 
            float lineWidth = 0.02f, bool major = false);
};

enum class NodeShape {
  CIRCLE,
  SQUARE,
  DIAMOND,
  TRIANGLE,
  HEXAGON
};

class CMapNode : public Component {
public:
  int nodeId;                    // Unique identifier for this node
  Vec2f position;               // Actual position (not grid-based)
  NodeShape shape;              // Shape of the node
  bool isSelected;              // Current selection state
  bool isNavigable;             // Whether this node can be navigated to
  sf::Color baseColor;          // Default color when not selected
  sf::Color selectedColor;      // Color when selected
  sf::Color pulseColor;         // Color for pulsing animation
  float pulseTimer;             // Animation timer for pulsing
  float size;                   // Size/radius of the node
  
  CMapNode();
  CMapNode(int id, const Vec2f& pos, NodeShape nodeShape = NodeShape::CIRCLE, 
           bool navigable = true, float nodeSize = 20.0f);
};

class CVoronoiRegion : public Component {
public:
  int regionId;
  Vec2f centroid;                      // Navigation center
  std::vector<Vec2f> originalVertices; // Clean Voronoi boundary
  std::vector<Vec2f> distortedBoundary; // Noise-distorted boundary
  std::vector<int> neighborIds;        // Adjacent regions
  float area;
  bool isSelected = false;
  bool isNavigable = true;
  
  // Visual properties
  sf::Color baseColor;
  sf::Color selectedColor;
  sf::Color borderColor;
  float pulseTimer = 0.0f;
  
  // Fantasy theming
  std::string regionName;
  std::string regionType;  // "Forest", "Mountains", "Plains", etc.
  
  CVoronoiRegion();
  CVoronoiRegion(int id, const Vec2f& center, const std::vector<Vec2f>& vertices);
  
  // Utility methods
  bool containsPoint(const Vec2f& point) const;
  void calculateCentroid();
  void calculateArea();
  Vec2f getClosestBoundaryPoint(const Vec2f& point) const;
  
private:
  bool pointInPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon) const;
};

// Enhanced rendering component for complex shapes
class CComplexShape : public Component {
public:
  enum ShapeType { CIRCLE, POLYGON, VORONOI_REGION };
  
  ShapeType type;
  std::vector<Vec2f> vertices;    // For polygons/regions
  sf::Color fillColor;
  sf::Color outlineColor;
  float outlineThickness;
  bool showVertices = false;      // Debug mode
  
  CComplexShape();
  CComplexShape(const std::vector<Vec2f>& verts, sf::Color fill, sf::Color outline, float thickness);
};
