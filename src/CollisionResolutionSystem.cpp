#include "../include/CollisionResolutionSystem.hpp"
#include "../include/Logger.hpp"
#include <glm/glm.hpp>

void CollisionResolutionSystem::resolveCollisions(const std::vector<CollisionEvent>& collisions) {
    LOG_DEBUG_STREAM("CollisionResolutionSystem: Resolving " << collisions.size() << " collisions");
    
    for (const auto& collision : collisions) {
        resolveCollision(collision);
    }
}

void CollisionResolutionSystem::resolveCollision(const CollisionEvent& collision) {
    // First, separate the entities to prevent overlapping
    separateEntities(collision);
    
    // Get response type for the collision
    CollisionResponse responseA = getResponseForEntity(collision.entityA);
    CollisionResponse responseB = getResponseForEntity(collision.entityB);
    
    // Use the more restrictive response type
    CollisionResponse effectiveResponse = responseA;
    if (responseB.type == CollisionResponseType::ABSORB) {
        effectiveResponse = responseB;
    } else if (responseA.type == CollisionResponseType::PASS_THROUGH && 
               responseB.type != CollisionResponseType::PASS_THROUGH) {
        effectiveResponse = responseB;
    }
    
    // Apply the appropriate response
    switch (effectiveResponse.type) {
        case CollisionResponseType::ELASTIC:
            applyElasticResponse(collision, effectiveResponse);
            break;
        case CollisionResponseType::DAMPED:
            applyDampedResponse(collision, effectiveResponse);
            break;
        case CollisionResponseType::ABSORB:
            applyAbsorbResponse(collision);
            break;
        case CollisionResponseType::PASS_THROUGH:
            // Do nothing - entities pass through each other
            break;
    }
    
    LOG_DEBUG_STREAM("CollisionResolutionSystem: Resolved collision between entities " 
                    << collision.entityA->id() << " and " << collision.entityB->id());
}

void CollisionResolutionSystem::setDefaultResponse(CollisionResponseType type, float restitution, float friction) {
    m_defaultResponse = CollisionResponse(type, restitution, friction);
}

void CollisionResolutionSystem::setEntityResponse(EntityTag tag, const CollisionResponse& response) {
    m_entityResponses[tag] = response;
}

CollisionResponse CollisionResolutionSystem::getResponseForEntity(std::shared_ptr<Entity> entity) {
    auto it = m_entityResponses.find(entity->tag());
    if (it != m_entityResponses.end()) {
        return it->second;
    }
    return m_defaultResponse;
}

void CollisionResolutionSystem::separateEntities(const CollisionEvent& collision) {
    // Move entities apart along the contact normal to prevent overlap
    if (!collision.entityA->has<CTransform3D>() || !collision.entityB->has<CTransform3D>()) {
        return;
    }
    
    auto& transformA = collision.entityA->get<CTransform3D>();
    auto& transformB = collision.entityB->get<CTransform3D>();
    
    // Move each entity half the penetration distance away from each other
    glm::vec3 separation = collision.contactNormal * (collision.penetrationDepth * 0.5f);
    
    transformA.position -= separation;
    transformB.position += separation;
}

void CollisionResolutionSystem::applyElasticResponse(const CollisionEvent& collision, const CollisionResponse& response) {
    if (!collision.entityA->has<CMovement3D>() || !collision.entityB->has<CMovement3D>()) {
        return;
    }
    
    auto& movementA = collision.entityA->get<CMovement3D>();
    auto& movementB = collision.entityB->get<CMovement3D>();
    
    // Calculate relative velocity
    glm::vec3 relativeVelocity = movementB.vel - movementA.vel;
    
    // Calculate relative velocity in collision normal direction
    float velocityAlongNormal = glm::dot(relativeVelocity, collision.contactNormal);
    
    // Do not resolve if velocities are separating
    if (velocityAlongNormal > 0) {
        return;
    }
    
    // Calculate impulse scalar
    float impulse = -(1 + response.restitution) * velocityAlongNormal;
    // Assuming equal mass for simplicity - could be enhanced with mass component
    impulse /= 2.0f;
    
    // Apply impulse
    glm::vec3 impulseVector = impulse * collision.contactNormal;
    movementA.vel -= impulseVector;
    movementB.vel += impulseVector;
}

void CollisionResolutionSystem::applyDampedResponse(const CollisionEvent& collision, const CollisionResponse& response) {
    if (!collision.entityA->has<CMovement3D>() || !collision.entityB->has<CMovement3D>()) {
        return;
    }
    
    auto& movementA = collision.entityA->get<CMovement3D>();
    auto& movementB = collision.entityB->get<CMovement3D>();
    
    // Calculate relative velocity
    glm::vec3 relativeVelocity = movementB.vel - movementA.vel;
    float velocityAlongNormal = glm::dot(relativeVelocity, collision.contactNormal);
    
    // Do not resolve if velocities are separating
    if (velocityAlongNormal > 0) {
        return;
    }
    
    // Apply damped response (similar to elastic but with energy loss)
    float dampedRestitution = response.restitution * 0.9f; // Additional damping
    float impulse = -(1 + dampedRestitution) * velocityAlongNormal / 2.0f;
    
    glm::vec3 impulseVector = impulse * collision.contactNormal;
    movementA.vel -= impulseVector;
    movementB.vel += impulseVector;
    
    // Apply friction to tangential velocity
    glm::vec3 tangentialVelocity = relativeVelocity - velocityAlongNormal * collision.contactNormal;
    if (glm::length(tangentialVelocity) > 0.01f) {
        glm::vec3 frictionForce = -glm::normalize(tangentialVelocity) * response.friction * std::abs(impulse);
        movementA.vel -= frictionForce * 0.5f;
        movementB.vel += frictionForce * 0.5f;
    }
}

void CollisionResolutionSystem::applyAbsorbResponse(const CollisionEvent& collision) {
    if (!collision.entityA->has<CMovement3D>() || !collision.entityB->has<CMovement3D>()) {
        return;
    }
    
    auto& movementA = collision.entityA->get<CMovement3D>();
    auto& movementB = collision.entityB->get<CMovement3D>();
    
    // Stop both entities at the collision point
    movementA.vel = glm::vec3(0.0f);
    movementB.vel = glm::vec3(0.0f);
    movementA.acc = glm::vec3(0.0f);
    movementB.acc = glm::vec3(0.0f);
}