#include "../include/CollisionSystem.hpp"
#include "../include/Logger.hpp"

CollisionSystem::CollisionSystem(PartitionType type,
                                const glm::vec3& worldMin,
                                const glm::vec3& worldMax,
                                float cellSize)
    : m_worldMin(worldMin), m_worldMax(worldMax) {
    
    m_spatialPartition = createSpatialPartition(type, worldMin, worldMax, cellSize);
    
    if (!m_spatialPartition) {
        LOG_ERROR("CollisionSystem: Failed to create spatial partition strategy");
        return;
    }
    
    LOG_INFO_STREAM("CollisionSystem: Initialized with " << m_spatialPartition->getStrategyName() 
              << " strategy");
}

void CollisionSystem::updateEntities(EntityManager& entityManager) {
    if (!m_spatialPartition) return;
    
    // Clear previous frame's data
    m_spatialPartition->clear();
    
    // Add all entities with collision components to spatial partition
    for (auto& entity : entityManager.getEntities()) {
        if (entity->has<CAABB>() && entity->has<CTransform3D>()) {
            CAABB worldAABB = AABBUtils::getWorldAABB(entity);
            
            // Only add entities that are within world bounds
            // This prevents issues with entities far outside the grid
            if (worldAABB.min.x >= m_worldMin.x && worldAABB.max.x <= m_worldMax.x &&
                worldAABB.min.y >= m_worldMin.y && worldAABB.max.y <= m_worldMax.y &&
                worldAABB.min.z >= m_worldMin.z && worldAABB.max.z <= m_worldMax.z) {
                
                m_spatialPartition->insert(entity->id(), worldAABB);
            }
        }
    }
}

std::vector<CollisionPair> CollisionSystem::findCollisions() {
    if (!m_spatialPartition) {
        return {};
    }
    
    return m_spatialPartition->findCollisions();
}

std::vector<std::shared_ptr<Entity>> CollisionSystem::findCollisionsFor(
    const std::shared_ptr<Entity>& entity, EntityManager& entityManager) {
    
    std::vector<std::shared_ptr<Entity>> result;
    
    if (!m_spatialPartition || !entity->has<CAABB>() || !entity->has<CTransform3D>()) {
        return result;
    }
    
    CAABB entityAABB = AABBUtils::getWorldAABB(entity);
    std::vector<EntityID> candidates = m_spatialPartition->query(entityAABB);
    
    // Convert EntityIDs to Entity objects and perform actual collision tests
    for (EntityID candidateId : candidates) {
        if (candidateId == entity->id()) continue; // Skip self
        
        auto candidate = entityManager.getEntityById(candidateId);
        if (candidate && candidate->has<CAABB>() && candidate->has<CTransform3D>()) {
            CAABB candidateAABB = AABBUtils::getWorldAABB(candidate);
            
            if (AABBUtils::intersects(entityAABB, candidateAABB)) {
                result.push_back(candidate);
            }
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Entity>> CollisionSystem::queryRegion(
    const CAABB& region, EntityManager& entityManager) {
    
    std::vector<std::shared_ptr<Entity>> result;
    
    if (!m_spatialPartition) {
        return result;
    }
    
    std::vector<EntityID> candidates = m_spatialPartition->query(region);
    
    // Convert EntityIDs to Entity objects
    for (EntityID entityId : candidates) {
        auto entity = entityManager.getEntityById(entityId);
        if (entity) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<Entity>> CollisionSystem::queryRadius(
    const glm::vec3& point, float radius, EntityManager& entityManager) {
    
    std::vector<std::shared_ptr<Entity>> result;
    
    if (!m_spatialPartition) {
        return result;
    }
    
    std::vector<EntityID> candidates = m_spatialPartition->queryRadius(point, radius);
    
    // Convert EntityIDs to Entity objects
    for (EntityID entityId : candidates) {
        auto entity = entityManager.getEntityById(entityId);
        if (entity) {
            result.push_back(entity);
        }
    }
    
    return result;
}

PartitionStats CollisionSystem::getStatistics() const {
    PartitionStats stats;
    
    if (m_spatialPartition) {
        m_spatialPartition->getStatistics(stats);
    }
    
    return stats;
}

void CollisionSystem::clear() {
    if (m_spatialPartition) {
        m_spatialPartition->clear();
    }
}

bool CollisionSystem::isValid() const {
    return m_spatialPartition && m_spatialPartition->isValid();
}