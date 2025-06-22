#include "../include/Component.h"
#include <limits>
#include <cmath>
#include <string>

CTransform::CTransform() = default;
CTransform::CTransform(const Vec2f &p, const Vec2f &v, float a)
    : pos(p), vel(v), angle(a){};

CTransform3D::CTransform3D() = default;
CTransform3D::CTransform3D(const glm::vec3 &position, const glm::vec3 &rotation,
                           const glm::vec3 &scale)
    : position(position), rotation(rotation), scale(scale){};

CShape::CShape() = default;
CShape::CShape(float radius, size_t points, const sf::Color &fill,
               const sf::Color &outline, float thickness)
    : circle(radius, points) {
  circle.setFillColor(fill);
  circle.setOutlineColor(outline);
  circle.setOutlineThickness(thickness);
  circle.setOrigin(radius, radius);
};

CCollision::CCollision() = default;
CCollision::CCollision(float r) : radius(r){};

CScore::CScore() = default;
CScore::CScore(int s) : score(s){};

CLifespan::CLifespan() = default;
CLifespan::CLifespan(int totalLifespan)
    : lifespan(totalLifespan), remaining(totalLifespan){};

CInput::CInput() = default;

CBoundingBox::CBoundingBox(const Vec2f &size)
    : size(size), halfsize(Vec2f{size.x / 2.0f, size.y / 2.0f}){};

CGravity::CGravity() = default;

CTriangle::CTriangle() = default;

CAABB::CAABB() = default;
CAABB::CAABB(const glm::vec3 &center, const glm::vec3 &halfsize) {
  min = center - halfsize;
  max = center + halfsize;
}

CMovement3D::CMovement3D() : vel(0.0f), acc(0.0f) {}

CMovement3D::CMovement3D(const glm::vec3 &velocity,
                         const glm::vec3 &acceleration)
    : vel(velocity), acc(acceleration){};

CSelection::CSelection() = default;
CSelection::CSelection(Vec2i pos) : grid_position(pos){};

CGridLine::CGridLine() : start(0.0f), end(0.0f), color(0.0f), width(0.0f), isMajor(false) {}
CGridLine::CGridLine(const glm::vec3& startPos, const glm::vec3& endPos, 
                     const glm::vec3& lineColor, float lineWidth, bool major)
    : start(startPos), end(endPos), color(lineColor), width(lineWidth), isMajor(major) {};

CMapNode::CMapNode() 
    : nodeId(-1), position(0.0f, 0.0f), shape(NodeShape::CIRCLE), isSelected(false), 
      isNavigable(true), baseColor(sf::Color::White), selectedColor(sf::Color::Yellow),
      pulseColor(sf::Color::Red), pulseTimer(0.0f), size(20.0f) {}

CMapNode::CMapNode(int id, const Vec2f& pos, NodeShape nodeShape, bool navigable, float nodeSize)
    : nodeId(id), position(pos), shape(nodeShape), isSelected(false), isNavigable(navigable),
      baseColor(sf::Color::White), selectedColor(sf::Color::Yellow), 
      pulseColor(sf::Color::Red), pulseTimer(0.0f), size(nodeSize) {};

CVoronoiRegion::CVoronoiRegion() 
    : regionId(-1), centroid(0.0f, 0.0f), area(0.0f), isSelected(false), isNavigable(true),
      baseColor(sf::Color::White), selectedColor(sf::Color::Yellow), borderColor(sf::Color::Black),
      pulseTimer(0.0f), regionName("Unknown"), regionType("Unknown") {}

CVoronoiRegion::CVoronoiRegion(int id, const Vec2f& center, const std::vector<Vec2f>& vertices)
    : regionId(id), centroid(center), originalVertices(vertices), area(0.0f), 
      isSelected(false), isNavigable(true), baseColor(sf::Color::White), 
      selectedColor(sf::Color::Yellow), borderColor(sf::Color::Black), pulseTimer(0.0f),
      regionName("Region " + std::to_string(id)), regionType("Grassland") {
    calculateArea();
    calculateCentroid();
}

bool CVoronoiRegion::containsPoint(const Vec2f& point) const {
    return pointInPolygon(point, originalVertices);
}

void CVoronoiRegion::calculateCentroid() {
    if (originalVertices.empty()) {
        return;
    }
    
    float cx = 0.0f, cy = 0.0f;
    float signedArea = 0.0f;
    
    for (size_t i = 0; i < originalVertices.size(); ++i) {
        size_t j = (i + 1) % originalVertices.size();
        float cross = originalVertices[i].x * originalVertices[j].y - originalVertices[j].x * originalVertices[i].y;
        signedArea += cross;
        cx += (originalVertices[i].x + originalVertices[j].x) * cross;
        cy += (originalVertices[i].y + originalVertices[j].y) * cross;
    }
    
    signedArea *= 0.5f;
    if (std::abs(signedArea) > 0.001f) {
        cx /= (6.0f * signedArea);
        cy /= (6.0f * signedArea);
        centroid = Vec2f(cx, cy);
    } else {
        // Fallback to simple average
        cx = cy = 0.0f;
        for (const auto& vertex : originalVertices) {
            cx += vertex.x;
            cy += vertex.y;
        }
        if (!originalVertices.empty()) {
            centroid = Vec2f(cx / originalVertices.size(), cy / originalVertices.size());
        }
    }
}

void CVoronoiRegion::calculateArea() {
    if (originalVertices.size() < 3) {
        area = 0.0f;
        return;
    }
    
    float totalArea = 0.0f;
    for (size_t i = 0; i < originalVertices.size(); ++i) {
        size_t j = (i + 1) % originalVertices.size();
        totalArea += originalVertices[i].x * originalVertices[j].y - originalVertices[j].x * originalVertices[i].y;
    }
    area = std::abs(totalArea) * 0.5f;
}

Vec2f CVoronoiRegion::getClosestBoundaryPoint(const Vec2f& point) const {
    if (originalVertices.empty()) return point;
    
    Vec2f closest = originalVertices[0];
    float minDist = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < originalVertices.size(); ++i) {
        size_t j = (i + 1) % originalVertices.size();
        
        // Find closest point on edge from vertex i to vertex j
        Vec2f edge = Vec2f(originalVertices[j].x - originalVertices[i].x, 
                          originalVertices[j].y - originalVertices[i].y);
        Vec2f toPoint = Vec2f(point.x - originalVertices[i].x, point.y - originalVertices[i].y);
        
        float edgeLength = edge.x * edge.x + edge.y * edge.y;
        if (edgeLength > 0.001f) {
            float t = (toPoint.x * edge.x + toPoint.y * edge.y) / edgeLength;
            t = std::max(0.0f, std::min(1.0f, t));
            
            Vec2f closestOnEdge;
            closestOnEdge.x = originalVertices[i].x + t * edge.x;
            closestOnEdge.y = originalVertices[i].y + t * edge.y;
            
            float dist = (point.x - closestOnEdge.x) * (point.x - closestOnEdge.x) + 
                        (point.y - closestOnEdge.y) * (point.y - closestOnEdge.y);
            
            if (dist < minDist) {
                minDist = dist;
                closest = closestOnEdge;
            }
        }
    }
    
    return closest;
}

bool CVoronoiRegion::pointInPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon) const {
    if (polygon.size() < 3) return false;
    
    bool inside = false;
    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / 
                      (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

CComplexShape::CComplexShape() 
    : type(CIRCLE), fillColor(sf::Color::White), outlineColor(sf::Color::Black), 
      outlineThickness(1.0f), showVertices(false) {}

CComplexShape::CComplexShape(const std::vector<Vec2f>& verts, sf::Color fill, sf::Color outline, float thickness)
    : type(POLYGON), vertices(verts), fillColor(fill), outlineColor(outline), 
      outlineThickness(thickness), showVertices(false) {};
