#include "../include/VoronoiGenerator.h"
#include "../include/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <limits>

// VoronoiCell implementation
bool VoronoiCell::containsPoint(const Vec2f& point) const {
    return pointInPolygon(point, vertices);
}

void VoronoiCell::calculateCentroid() {
    if (vertices.empty()) {
        centroid = site;
        return;
    }
    
    float cx = 0.0f, cy = 0.0f;
    float signedArea = 0.0f;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t j = (i + 1) % vertices.size();
        float cross = vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y;
        signedArea += cross;
        cx += (vertices[i].x + vertices[j].x) * cross;
        cy += (vertices[i].y + vertices[j].y) * cross;
    }
    
    signedArea *= 0.5f;
    if (std::abs(signedArea) > 0.001f) {
        cx /= (6.0f * signedArea);
        cy /= (6.0f * signedArea);
        centroid = Vec2f(cx, cy);
    } else {
        // Fallback to simple average
        cx = cy = 0.0f;
        for (const auto& vertex : vertices) {
            cx += vertex.x;
            cy += vertex.y;
        }
        centroid = Vec2f(cx / vertices.size(), cy / vertices.size());
    }
}

void VoronoiCell::calculateArea() {
    if (vertices.size() < 3) {
        area = 0.0f;
        return;
    }
    
    float totalArea = 0.0f;
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t j = (i + 1) % vertices.size();
        totalArea += vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y;
    }
    area = std::abs(totalArea) * 0.5f;
}

bool VoronoiCell::pointInPolygon(const Vec2f& point, const std::vector<Vec2f>& polygon) const {
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

// VoronoiGenerator implementation
void VoronoiGenerator::generateRandomSites(int count, float minDistance, unsigned int seed) {
    m_rng.seed(seed);
    m_sites.clear();
    m_sites.reserve(count);
    
    LOG_INFO_STREAM("VoronoiGenerator: Generating " << count << " random sites with minimum distance " << minDistance);
    
    // Use Poisson disk sampling for better distribution
    int maxAttempts = count * 10;
    int attempts = 0;
    
    while (m_sites.size() < static_cast<size_t>(count) && attempts < maxAttempts) {
        Vec2f newSite = generateRandomPoint();
        
        if (!isSiteTooClose(newSite, m_sites, minDistance)) {
            VoronoiSite site(newSite, m_sites.size());
            m_sites.push_back(site);
            LOG_DEBUG_STREAM("VoronoiGenerator: Added site " << m_sites.size() - 1 << " at (" 
                           << newSite.x << ", " << newSite.y << ")");
        }
        attempts++;
    }
    
    LOG_INFO_STREAM("VoronoiGenerator: Generated " << m_sites.size() << " sites in " << attempts << " attempts");
}

void VoronoiGenerator::computeVoronoiDiagram() {
    LOG_INFO("VoronoiGenerator: Computing Voronoi diagram");
    
    m_cells.clear();
    m_cells.reserve(m_sites.size());
    
    // Create initial cells from sites
    for (const auto& site : m_sites) {
        VoronoiCell cell(site.id, site.position);
        m_cells.push_back(cell);
    }
    
    computeVoronoiCellsFromSites();
    clipToBounds();
    validateCells();
    generateNeighborhood();
    
    LOG_INFO_STREAM("VoronoiGenerator: Created " << m_cells.size() << " valid cells");
}

void VoronoiGenerator::relaxSites(int iterations) {
    LOG_INFO_STREAM("VoronoiGenerator: Performing " << iterations << " Lloyd relaxation iterations");
    
    for (int iter = 0; iter < iterations; ++iter) {
        LOG_DEBUG_STREAM("VoronoiGenerator: Relaxation iteration " << (iter + 1));
        
        // Calculate new site positions as centroids of their cells
        std::vector<Vec2f> newPositions;
        newPositions.reserve(m_cells.size());
        
        for (auto& cell : m_cells) {
            cell.calculateCentroid();
            Vec2f newPos = cell.centroid;
            
            // Keep sites within bounds
            newPos.x = std::max(10.0f, std::min(m_bounds.x - 10.0f, newPos.x));
            newPos.y = std::max(10.0f, std::min(m_bounds.y - 10.0f, newPos.y));
            
            newPositions.push_back(newPos);
        }
        
        // Update site positions
        updateSitePositions(newPositions);
        
        // Recompute Voronoi diagram with new positions
        computeVoronoiCellsFromSites();
        clipToBounds();
        validateCells();
    }
    
    // Final neighborhood computation
    generateNeighborhood();
    LOG_INFO("VoronoiGenerator: Lloyd relaxation completed");
}

void VoronoiGenerator::computeVoronoiCellsFromSites() {
    // Simple implementation: for each site, create a cell by finding all points
    // closer to this site than to any other site
    
    const float step = 2.0f; // Sampling resolution
    const int width = static_cast<int>(m_bounds.x / step);
    const int height = static_cast<int>(m_bounds.y / step);
    
    for (auto& cell : m_cells) {
        cell.vertices.clear();
        std::vector<Vec2f> boundaryPoints;
        
        // Sample grid points and find boundary
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Vec2f point(x * step, y * step);
                
                // Find closest site to this point
                float minDist = std::numeric_limits<float>::max();
                int closestSite = -1;
                
                for (size_t i = 0; i < m_sites.size(); ++i) {
                    float dx = point.x - m_sites[i].position.x;
                    float dy = point.y - m_sites[i].position.y;
                    float dist = dx * dx + dy * dy;
                    
                    if (dist < minDist) {
                        minDist = dist;
                        closestSite = i;
                    }
                }
                
                // If this point belongs to current cell, check if it's on boundary
                if (closestSite == cell.cellId) {
                    bool isBoundary = false;
                    
                    // Check neighboring points
                    for (int dy = -1; dy <= 1 && !isBoundary; ++dy) {
                        for (int dx = -1; dx <= 1 && !isBoundary; ++dx) {
                            if (dx == 0 && dy == 0) continue;
                            
                            Vec2f neighbor(point.x + dx * step, point.y + dy * step);
                            if (neighbor.x < 0 || neighbor.x >= m_bounds.x || 
                                neighbor.y < 0 || neighbor.y >= m_bounds.y) {
                                isBoundary = true;
                                continue;
                            }
                            
                            // Find closest site to neighbor
                            float neighborMinDist = std::numeric_limits<float>::max();
                            int neighborClosestSite = -1;
                            
                            for (size_t i = 0; i < m_sites.size(); ++i) {
                                float ndx = neighbor.x - m_sites[i].position.x;
                                float ndy = neighbor.y - m_sites[i].position.y;
                                float ndist = ndx * ndx + ndy * ndy;
                                
                                if (ndist < neighborMinDist) {
                                    neighborMinDist = ndist;
                                    neighborClosestSite = i;
                                }
                            }
                            
                            if (neighborClosestSite != cell.cellId) {
                                isBoundary = true;
                            }
                        }
                    }
                    
                    if (isBoundary) {
                        boundaryPoints.push_back(point);
                    }
                }
            }
        }
        
        // Convert boundary points to ordered vertices
        if (!boundaryPoints.empty()) {
            cell.vertices = orderVerticesClockwise(boundaryPoints, cell.site);
            cell.calculateArea();
            cell.calculateCentroid();
        }
    }
}

std::vector<Vec2f> VoronoiGenerator::orderVerticesClockwise(const std::vector<Vec2f>& points, const Vec2f& center) {
    if (points.empty()) return {};
    
    // Sort points by angle from center
    std::vector<std::pair<float, Vec2f>> anglePoints;
    for (const auto& point : points) {
        float angle = std::atan2(point.y - center.y, point.x - center.x);
        anglePoints.push_back({angle, point});
    }
    
    // Sort by angle using custom comparator
    std::sort(anglePoints.begin(), anglePoints.end(), 
        [](const std::pair<float, Vec2f>& a, const std::pair<float, Vec2f>& b) {
            return a.first < b.first;
        });
    
    std::vector<Vec2f> orderedVertices;
    for (const auto& ap : anglePoints) {
        orderedVertices.push_back(ap.second);
    }
    
    return orderedVertices;
}

void VoronoiGenerator::updateSitePositions(const std::vector<Vec2f>& newPositions) {
    for (size_t i = 0; i < std::min(m_sites.size(), newPositions.size()); ++i) {
        m_sites[i].position = newPositions[i];
        if (i < m_cells.size()) {
            m_cells[i].site = newPositions[i];
        }
    }
}

void VoronoiGenerator::clipToBounds() {
    for (auto& cell : m_cells) {
        clipCellToBounds(cell);
    }
}

void VoronoiGenerator::clipCellToBounds(VoronoiCell& cell) {
    if (cell.vertices.empty()) return;
    
    std::vector<Vec2f> clippedVertices;
    
    for (const auto& vertex : cell.vertices) {
        if (isPointInBounds(vertex)) {
            clippedVertices.push_back(vertex);
        } else {
            // Clip to bounds (simplified - just clamp to bounds)
            Vec2f clampedVertex;
            clampedVertex.x = std::max(0.0f, std::min(m_bounds.x, vertex.x));
            clampedVertex.y = std::max(0.0f, std::min(m_bounds.y, vertex.y));
            clippedVertices.push_back(clampedVertex);
        }
    }
    
    cell.vertices = clippedVertices;
    cell.calculateArea();
    cell.calculateCentroid();
}

bool VoronoiGenerator::isPointInBounds(const Vec2f& point) const {
    return point.x >= 0 && point.x <= m_bounds.x && 
           point.y >= 0 && point.y <= m_bounds.y;
}

void VoronoiGenerator::validateCells() {
    auto it = std::remove_if(m_cells.begin(), m_cells.end(),
        [](const VoronoiCell& cell) { return !cell.isValidRegion(); });
    m_cells.erase(it, m_cells.end());
    
    LOG_DEBUG_STREAM("VoronoiGenerator: " << m_cells.size() << " cells remain after validation");
}

void VoronoiGenerator::generateNeighborhood() {
    for (auto& cell : m_cells) {
        cell.neighborIds.clear();
        findNeighborsForCell(cell);
    }
}

void VoronoiGenerator::findNeighborsForCell(VoronoiCell& cell) {
    for (const auto& otherCell : m_cells) {
        if (otherCell.cellId != cell.cellId && areCellsAdjacent(cell, otherCell)) {
            cell.neighborIds.push_back(otherCell.cellId);
        }
    }
}

bool VoronoiGenerator::areCellsAdjacent(const VoronoiCell& cell1, const VoronoiCell& cell2) const {
    // Check if cells share an edge (simplified check)
    const float tolerance = 5.0f;
    
    for (const auto& v1 : cell1.vertices) {
        for (const auto& v2 : cell2.vertices) {
            float dx = v1.x - v2.x;
            float dy = v1.y - v2.y;
            if (dx * dx + dy * dy < tolerance * tolerance) {
                return true;
            }
        }
    }
    return false;
}

bool VoronoiGenerator::isSiteTooClose(const Vec2f& newSite, const std::vector<VoronoiSite>& existing, float minDistance) {
    for (const auto& site : existing) {
        float dx = newSite.x - site.position.x;
        float dy = newSite.y - site.position.y;
        if (dx * dx + dy * dy < minDistance * minDistance) {
            return true;
        }
    }
    return false;
}

Vec2f VoronoiGenerator::generateRandomPoint() {
    std::uniform_real_distribution<float> xDist(0.0f, m_bounds.x);
    std::uniform_real_distribution<float> yDist(0.0f, m_bounds.y);
    return Vec2f(xDist(m_rng), yDist(m_rng));
}

void VoronoiGenerator::clear() {
    m_sites.clear();
    m_cells.clear();
    m_edges.clear();
}

bool VoronoiGenerator::isValidConfiguration() const {
    return !m_cells.empty() && m_cells.size() >= 3;
}