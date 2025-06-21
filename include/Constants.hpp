#pragma once

/**
 * Game Engine Constants
 * 
 * Centralized location for all engine configuration constants.
 * Organized by subsystem for easy maintenance and modification.
 */

namespace EngineConstants {

// ============================================================================
// Display and Window Configuration
// ============================================================================

namespace Display {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr int TARGET_FRAMERATE = 60;
    constexpr float ASPECT_RATIO = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
}

// ============================================================================
// Graphics and Rendering
// ============================================================================

namespace Graphics {
    // Shader compilation
    constexpr int SHADER_LOG_BUFFER_SIZE = 512;
    
    // Background clear color (dark grey-green)
    constexpr float CLEAR_COLOR_R = 0.2f;
    constexpr float CLEAR_COLOR_G = 0.3f;
    constexpr float CLEAR_COLOR_B = 0.3f;
    constexpr float CLEAR_COLOR_A = 1.0f;
    
    // Vertex buffer layout
    constexpr int TRIANGLE_VERTEX_DATA_SIZE = 18;  // 6 components × 3 vertices
    constexpr int VERTEX_STRIDE_SIZE = 6;          // position(3) + color(3)
    constexpr int COLOR_ATTRIBUTE_OFFSET = 3;      // Offset for color data in vertex
}

// ============================================================================
// Camera Configuration
// ============================================================================

namespace Camera {
    // Default camera orientation
    constexpr float DEFAULT_YAW = -90.0f;
    constexpr float DEFAULT_PITCH = 0.0f;
    constexpr float DEFAULT_FOV = 45.0f;
    
    // Projection matrix clipping planes
    constexpr float NEAR_CLIP_PLANE = 0.1f;
    constexpr float FAR_CLIP_PLANE = 100.0f;
    
    // Movement and input
    constexpr float MOVEMENT_SPEED = 2.5f;
    constexpr float MOUSE_SENSITIVITY = 0.05f;
    
    // Pitch angle constraints (prevent camera flipping)
    constexpr float MAX_PITCH_ANGLE = 89.0f;
    constexpr float MIN_PITCH_ANGLE = -89.0f;
    
    // Starting position in 3D space
    constexpr float START_X = 0.0f;
    constexpr float START_Y = 0.0f;
    constexpr float START_Z = 3.0f;
}

// ============================================================================
// Game World and Physics
// ============================================================================

namespace World {
    // World boundary limits for collision system
    constexpr float MIN_BOUND = -100.0f;
    constexpr float MAX_BOUND = 100.0f;
    
    // Collision response
    constexpr float COLLISION_DAMPING_FACTOR = -0.9f;
    
    // Entity spawning and layout
    constexpr int ENTITY_GRID_SIZE = 3;           // 3×3×3 grid of entities
    constexpr float ENTITY_SPACING_X = 2.0f;
    constexpr float ENTITY_SPACING_Y = 3.0f;
    constexpr float ENTITY_SPACING_Z = 4.0f;
    constexpr float ENTITY_ROTATION_RATE = 1.0f; // Rotation increment per frame
}

// ============================================================================
// Spatial Partitioning System
// ============================================================================

namespace SpatialPartition {
    // Default configuration
    constexpr float DEFAULT_CELL_SIZE = 10.0f;
    constexpr float DEFAULT_WORLD_MIN = -100.0f;
    constexpr float DEFAULT_WORLD_MAX = 100.0f;
    
    // Algorithm parameters
    constexpr int MIN_ENTITIES_FOR_COLLISION = 2;      // Skip collision check if < 2 entities
    constexpr int QUADTREE_MAX_ENTITIES_PER_NODE = 8;  // Max entities before subdivision
    constexpr float SPATIAL_HASH_DEFAULT_CELL_SIZE = 10.0f;
    
    // Geometric calculations
    constexpr float CENTER_CALCULATION_FACTOR = 0.5f;  // For computing AABB centers
    constexpr float HALF_EXTENTS_FACTOR = 0.5f;        // For computing half-extents
}

// ============================================================================
// User Interface and Input
// ============================================================================

namespace Input {
    // Mouse input thresholds
    constexpr int MOUSE_MOVEMENT_THRESHOLD = 2;  // Minimum pixels to register movement
}

namespace UI {
    // Map scene configuration
    constexpr int MAP_GRID_COLS = 10;
    constexpr int MAP_GRID_ROWS = 10;
    constexpr float MAP_NODE_SIZE_FACTOR = 0.4f;      // Node radius as fraction of grid cell
    constexpr int MAP_NODE_SHAPE_POINTS = 4;          // Makes CircleShape into a square
    constexpr float MAP_NODE_OUTLINE_THICKNESS = 3.0f;
    constexpr float GRID_CENTER_OFFSET = 0.5f;        // Position nodes at cell centers
    constexpr float MAP_NODE_ROTATION_ANGLE = 45.0f;  // Diamond rotation
}

// ============================================================================
// Physics Constants
// ============================================================================

namespace Physics {
    // Gravity acceleration
    constexpr float GRAVITY_X = 0.0f;
    constexpr float GRAVITY_Y = 9.8f;  // Standard Earth gravity (m/s²)
}

} // namespace EngineConstants