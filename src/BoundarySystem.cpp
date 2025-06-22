#include "../include/BoundarySystem.hpp"
#include "../include/Logger.hpp"
#include "../include/Constants.hpp"

BoundarySystem::BoundarySystem(const BoundaryConstraint& constraint) 
    : m_globalConstraint(constraint) {
}

void BoundarySystem::enforceBoundaries(EntityManager& entityManager) {
    m_entitiesToDestroy.clear();
    
    for (auto& entity : entityManager.getEntities()) {
        if (!entity->has<CTransform3D>()) {
            continue;
        }
        
        if (isEntityOutOfBounds(entity)) {
            glm::vec3 violations = getViolatedBoundaries(entity);
            handleBoundaryViolation(entity, violations);
        }
    }
    
    // Destroy entities marked for destruction
    for (auto& entity : m_entitiesToDestroy) {
        entity->destroy();
        LOG_DEBUG_STREAM("BoundarySystem: Destroyed entity " << entity->id() << " for boundary violation");
    }
    
    if (!m_entitiesToDestroy.empty()) {
        LOG_DEBUG_STREAM("BoundarySystem: Processed " << m_entitiesToDestroy.size() << " boundary violations");
    }
}

void BoundarySystem::setBoundaryConstraint(const BoundaryConstraint& constraint) {
    m_globalConstraint = constraint;
    LOG_DEBUG_STREAM("BoundarySystem: Updated global boundary constraint");
}

void BoundarySystem::setEntityBoundaryAction(EntityTag tag, BoundaryAction action, float damping) {
    m_entityActions[tag] = action;
    m_entityDamping[tag] = damping;
}

bool BoundarySystem::isEntityOutOfBounds(std::shared_ptr<Entity> entity) const {
    if (!entity->has<CTransform3D>()) {
        return false;
    }
    
    const auto& position = entity->get<CTransform3D>().position;
    
    return position.x < m_globalConstraint.minBounds.x || position.x > m_globalConstraint.maxBounds.x ||
           position.y < m_globalConstraint.minBounds.y || position.y > m_globalConstraint.maxBounds.y ||
           position.z < m_globalConstraint.minBounds.z || position.z > m_globalConstraint.maxBounds.z;
}

glm::vec3 BoundarySystem::getViolatedBoundaries(std::shared_ptr<Entity> entity) const {
    if (!entity->has<CTransform3D>()) {
        return glm::vec3(0.0f);
    }
    
    const auto& position = entity->get<CTransform3D>().position;
    glm::vec3 violations(0.0f);
    
    // Positive values indicate violation in positive direction, negative in negative direction
    if (position.x > m_globalConstraint.maxBounds.x) {
        violations.x = 1.0f;
    } else if (position.x < m_globalConstraint.minBounds.x) {
        violations.x = -1.0f;
    }
    
    if (position.y > m_globalConstraint.maxBounds.y) {
        violations.y = 1.0f;
    } else if (position.y < m_globalConstraint.minBounds.y) {
        violations.y = -1.0f;
    }
    
    if (position.z > m_globalConstraint.maxBounds.z) {
        violations.z = 1.0f;
    } else if (position.z < m_globalConstraint.minBounds.z) {
        violations.z = -1.0f;
    }
    
    return violations;
}

void BoundarySystem::handleBoundaryViolation(std::shared_ptr<Entity> entity, const glm::vec3& violations) {
    BoundaryAction action = getBoundaryActionForEntity(entity);
    float damping = getDampingForEntity(entity);
    
    switch (action) {
        case BoundaryAction::BOUNCE:
            applyBounceAction(entity, violations, damping);
            break;
        case BoundaryAction::WRAP:
            applyWrapAction(entity, violations);
            break;
        case BoundaryAction::CLAMP:
            applyClampAction(entity, violations);
            break;
        case BoundaryAction::DESTROY:
            m_entitiesToDestroy.push_back(entity);
            break;
    }
}

void BoundarySystem::applyBounceAction(std::shared_ptr<Entity> entity, const glm::vec3& violations, float damping) {
    auto& transform = entity->get<CTransform3D>();
    
    // Clamp position to boundaries
    if (violations.x > 0) {
        transform.position.x = m_globalConstraint.maxBounds.x;
    } else if (violations.x < 0) {
        transform.position.x = m_globalConstraint.minBounds.x;
    }
    
    if (violations.y > 0) {
        transform.position.y = m_globalConstraint.maxBounds.y;
    } else if (violations.y < 0) {
        transform.position.y = m_globalConstraint.minBounds.y;
    }
    
    if (violations.z > 0) {
        transform.position.z = m_globalConstraint.maxBounds.z;
    } else if (violations.z < 0) {
        transform.position.z = m_globalConstraint.minBounds.z;
    }
    
    // Reverse and damp velocity for bouncing
    if (entity->has<CMovement3D>()) {
        auto& movement = entity->get<CMovement3D>();
        
        if (violations.x != 0) {
            movement.vel.x *= -damping;
        }
        if (violations.y != 0) {
            movement.vel.y *= -damping;
        }
        if (violations.z != 0) {
            movement.vel.z *= -damping;
        }
    }
}

void BoundarySystem::applyWrapAction(std::shared_ptr<Entity> entity, const glm::vec3& violations) {
    auto& transform = entity->get<CTransform3D>();
    
    // Teleport to opposite side
    if (violations.x > 0) {
        transform.position.x = m_globalConstraint.minBounds.x;
    } else if (violations.x < 0) {
        transform.position.x = m_globalConstraint.maxBounds.x;
    }
    
    if (violations.y > 0) {
        transform.position.y = m_globalConstraint.minBounds.y;
    } else if (violations.y < 0) {
        transform.position.y = m_globalConstraint.maxBounds.y;
    }
    
    if (violations.z > 0) {
        transform.position.z = m_globalConstraint.minBounds.z;
    } else if (violations.z < 0) {
        transform.position.z = m_globalConstraint.maxBounds.z;
    }
}

void BoundarySystem::applyClampAction(std::shared_ptr<Entity> entity, const glm::vec3& violations) {
    auto& transform = entity->get<CTransform3D>();
    
    // Clamp position to boundaries
    transform.position.x = glm::clamp(transform.position.x, 
                                     m_globalConstraint.minBounds.x, 
                                     m_globalConstraint.maxBounds.x);
    transform.position.y = glm::clamp(transform.position.y, 
                                     m_globalConstraint.minBounds.y, 
                                     m_globalConstraint.maxBounds.y);
    transform.position.z = glm::clamp(transform.position.z, 
                                     m_globalConstraint.minBounds.z, 
                                     m_globalConstraint.maxBounds.z);
    
    // Stop velocity in violated directions
    if (entity->has<CMovement3D>()) {
        auto& movement = entity->get<CMovement3D>();
        
        if (violations.x != 0) {
            movement.vel.x = 0.0f;
        }
        if (violations.y != 0) {
            movement.vel.y = 0.0f;
        }
        if (violations.z != 0) {
            movement.vel.z = 0.0f;
        }
    }
}

BoundaryAction BoundarySystem::getBoundaryActionForEntity(std::shared_ptr<Entity> entity) const {
    auto it = m_entityActions.find(entity->tag());
    if (it != m_entityActions.end()) {
        return it->second;
    }
    return m_globalConstraint.action;
}

float BoundarySystem::getDampingForEntity(std::shared_ptr<Entity> entity) const {
    auto it = m_entityDamping.find(entity->tag());
    if (it != m_entityDamping.end()) {
        return it->second;
    }
    return m_globalConstraint.damping;
}