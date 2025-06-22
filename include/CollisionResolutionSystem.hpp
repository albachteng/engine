#pragma once

#include "CollisionDetectionSystem.hpp"
#include "Component.h"
#include <vector>
#include <memory>

enum class CollisionResponseType {
    ELASTIC,    // Perfect bounce
    DAMPED,     // Bounce with energy loss
    ABSORB,     // Stop on contact
    PASS_THROUGH // No physical response
};

struct CollisionResponse {
    CollisionResponseType type;
    float restitution;  // Bounce factor (0.0 = no bounce, 1.0 = perfect bounce)
    float friction;     // Surface friction coefficient
    
    CollisionResponse(CollisionResponseType t = CollisionResponseType::DAMPED, 
                     float r = 0.9f, float f = 0.1f)
        : type(t), restitution(r), friction(f) {}
};

class CollisionResolutionSystem {
public:
    CollisionResolutionSystem() = default;
    ~CollisionResolutionSystem() = default;

    void resolveCollisions(const std::vector<CollisionEvent>& collisions);
    
    void resolveCollision(const CollisionEvent& collision);
    
    void setDefaultResponse(CollisionResponseType type, float restitution = 0.9f, float friction = 0.1f);
    
    void setEntityResponse(EntityTag tag, const CollisionResponse& response);

private:
    CollisionResponse getResponseForEntity(std::shared_ptr<Entity> entity);
    
    void separateEntities(const CollisionEvent& collision);
    
    void applyElasticResponse(const CollisionEvent& collision, const CollisionResponse& response);
    
    void applyDampedResponse(const CollisionEvent& collision, const CollisionResponse& response);
    
    void applyAbsorbResponse(const CollisionEvent& collision);
    
    CollisionResponse m_defaultResponse;
    std::unordered_map<EntityTag, CollisionResponse> m_entityResponses;
};