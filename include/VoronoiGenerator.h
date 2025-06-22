#pragma once
#include "Component.h"
#include <vector>
#include <memory>
#include <random>
#include <string>

struct VoronoiSite {
    Vec2f position;
    int id;
    sf::Color color;
    
    VoronoiSite() = default;
    VoronoiSite(Vec2f pos, int identifier) : position(pos), id(identifier) {}
};

struct VoronoiEdge {
    Vec2f start, end;
    int leftSite, rightSite;
    
    VoronoiEdge() = default;
    VoronoiEdge(Vec2f s, Vec2f e, int left = -1, int right = -1) 
        : start(s), end(e), leftSite(left), rightSite(right) {}
};

class VoronoiCell {
public:
    int cellId;
    Vec2f site;                          // Original seed point
    Vec2f centroid;                      // Center of mass for navigation
    std::vector<Vec2f> vertices;         // Polygon vertices (clockwise)
    std::vector<Vec2f> noisyBoundary;    // Noise-distorted boundary
    std::vector<int> neighborIds;        // Adjacent cell IDs
    float area;
    bool isNavigable = true;
    sf::Color regionColor;
    sf::Color borderColor;
    std::string regionName;              // Optional fantasy name
    
    VoronoiCell() = default;
    VoronoiCell(int id, Vec2f sitePos) : cellId(id), site(sitePos), area(0.0f) {}
    
    // Utility methods
    bool containsPoint(const Vec2f& point) const;
    void calculateCentroid();
    void calculateArea();
    bool isValidRegion() const { return vertices.size() >= 3 && area > 100.0f; }
    
private:
    bool pointInPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon) const;
};

class VoronoiGenerator {
private:
    std::vector<VoronoiSite> m_sites;
    std::vector<VoronoiCell> m_cells;
    std::vector<VoronoiEdge> m_edges;
    Vec2f m_bounds;
    std::mt19937 m_rng;
    
public:
    VoronoiGenerator(Vec2f bounds) : m_bounds(bounds) {}
    
    // Main generation pipeline using Lloyd Relaxation
    void generateRandomSites(int count, float minDistance, unsigned int seed);
    void computeVoronoiDiagram();
    void relaxSites(int iterations = 2);  // Lloyd relaxation for better distribution
    void clipToBounds();
    void validateCells();
    void generateNeighborhood();
    
    // Accessors
    const std::vector<VoronoiCell>& getCells() const { return m_cells; }
    const std::vector<VoronoiEdge>& getEdges() const { return m_edges; }
    const std::vector<VoronoiSite>& getSites() const { return m_sites; }
    std::vector<VoronoiCell>& getCellsMutable() { return m_cells; }
    
    // Utility
    void clear();
    int getCellCount() const { return m_cells.size(); }
    bool isValidConfiguration() const;
    
private:
    // Lloyd Relaxation implementation
    void computeVoronoiCellsFromSites();
    Vec2f relaxSite(const VoronoiCell& cell);
    void updateSitePositions(const std::vector<Vec2f>& newPositions);
    
    // Voronoi computation helpers
    void createBoundingBox();
    void computeCellVertices(VoronoiCell& cell);
    std::vector<Vec2f> findCellVerticesForSite(const Vec2f& site);
    
    // Clipping and validation
    void clipCellToBounds(VoronoiCell& cell);
    bool isPointInBounds(const Vec2f& point) const;
    void removeDegenerateCells();
    
    // Neighborhood computation
    void findNeighborsForCell(VoronoiCell& cell);
    bool areCellsAdjacent(const VoronoiCell& cell1, const VoronoiCell& cell2) const;
    
    // Site generation helpers
    bool isSiteTooClose(const Vec2f& newSite, const std::vector<VoronoiSite>& existing, float minDistance);
    Vec2f generateRandomPoint();
    
    // Vertex ordering
    std::vector<Vec2f> orderVerticesClockwise(const std::vector<Vec2f>& points, const Vec2f& center);
};