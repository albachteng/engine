#include <iostream>
#include <glm/glm.hpp>

// Simple test to check boundary logic without dependencies
struct MockBoundaryConstraint {
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    MockBoundaryConstraint(glm::vec3 min, glm::vec3 max) : minBounds(min), maxBounds(max) {}
};

bool isOutOfBounds(glm::vec3 position, const MockBoundaryConstraint& constraint) {
    return position.x < constraint.minBounds.x || position.x > constraint.maxBounds.x ||
           position.y < constraint.minBounds.y || position.y > constraint.maxBounds.y ||
           position.z < constraint.minBounds.z || position.z > constraint.maxBounds.z;
}

glm::vec3 getViolations(glm::vec3 position, const MockBoundaryConstraint& constraint) {
    glm::vec3 violations(0.0f);
    
    if (position.x > constraint.maxBounds.x) {
        violations.x = 1.0f;
    } else if (position.x < constraint.minBounds.x) {
        violations.x = -1.0f;
    }
    
    if (position.y > constraint.maxBounds.y) {
        violations.y = 1.0f;
    } else if (position.y < constraint.minBounds.y) {
        violations.y = -1.0f;
    }
    
    if (position.z > constraint.maxBounds.z) {
        violations.z = 1.0f;
    } else if (position.z < constraint.minBounds.z) {
        violations.z = -1.0f;
    }
    
    return violations;
}

int main() {
    MockBoundaryConstraint constraint(glm::vec3(-10.0f), glm::vec3(10.0f));
    glm::vec3 position(15.0f, 0.0f, 0.0f);
    
    std::cout << "Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
    std::cout << "Bounds: [" << constraint.minBounds.x << " to " << constraint.maxBounds.x << "]" << std::endl;
    std::cout << "Is out of bounds: " << isOutOfBounds(position, constraint) << std::endl;
    
    auto violations = getViolations(position, constraint);
    std::cout << "Violations: " << violations.x << ", " << violations.y << ", " << violations.z << std::endl;
    
    return 0;
}