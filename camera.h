// camera.h : orbit camera and the projection from world space to screen space.
// The maths is the same as the old desk-lamp viewer, but the result is returned
// instead of being sent to OpenGL, so the rasterizer can use it.
#pragma once
#include "vec3.h"

// Result of projecting one world-space point.
//   x, y  : screen position in [-1, 1] for a point inside the view (y up)
//   z     : value for the Z-buffer, smaller = closer
//   eyeDepth : distance in front of the eye
//   visible  : false if the point is behind (or too close to) the eye
struct ScreenPoint {
    float x = 0, y = 0, z = 0;
    float eyeDepth = 0;
    bool visible = false;
};

struct Camera {
    float phiDegrees = 50.0f;     // angle around the Z axis
    float thetaDegrees = 30.0f;   // angle above the ground
    float distance = 12.0f;       // eye distance from lookAt
    Vec3 lookAt = { 0.0f, 0.0f, 0.0f };

    float projectionPlaneDistance = 4.0f;   // "D": bigger = magnified view
    float screenScale = 0.6f;
    bool useParallelProjection = false;

    // Filled in by update():
    Vec3 eye = { 0, 0, 0 };
    Vec3 right = { 1, 0, 0 }, up = { 0, 0, 1 }, forward = { 0, 1, 0 };

    static constexpr float STARTING_PROJECTION_PLANE_DISTANCE = 4.0f;
    static constexpr float PARALLEL_SCALE = 0.2f;   // makes parallel match perspective at distance 12
    static constexpr float NEAR_PLANE = 0.1f;
    static constexpr float MAX_THETA_DEGREES = 89.0f;

    // Call after changing phi, theta, distance or lookAt.
    void update();

    ScreenPoint project(Vec3 worldPoint) const;
};