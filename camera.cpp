#include "camera.h"

void Camera::update() {
    float phi = phiDegrees * DEGREES_TO_RADIANS;
    float theta = thetaDegrees * DEGREES_TO_RADIANS;
    float cosPhi = std::cos(phi), sinPhi = std::sin(phi);
    float cosTheta = std::cos(theta), sinTheta = std::sin(theta);

    eye = { lookAt.x + distance * cosTheta * cosPhi,
            lookAt.y + distance * cosTheta * sinPhi,
            lookAt.z + distance * sinTheta };
    forward = { -cosTheta * cosPhi, -cosTheta * sinPhi, -sinTheta };
    right = { -sinPhi, cosPhi, 0.0f };
    up = { -sinTheta * cosPhi, -sinTheta * sinPhi, cosTheta };
}

ScreenPoint Camera::project(Vec3 worldPoint) const {
    Vec3 fromEye = worldPoint - eye;
    float eyeX = dot(right, fromEye);
    float eyeY = dot(up, fromEye);
    float depth = dot(forward, fromEye);

    ScreenPoint result;
    result.eyeDepth = depth;

    if (useParallelProjection) {
        // Ignore depth for x and y, so size does not depend on distance.
        float scale = PARALLEL_SCALE * projectionPlaneDistance / STARTING_PROJECTION_PLANE_DISTANCE;
        result.x = eyeX * scale;
        result.y = eyeY * scale;
        result.z = depth * 0.02f;
        result.visible = true;
    }
    else {
        // Divide by depth, so far things look smaller.
        if (depth < NEAR_PLANE) return result;   // behind the eye: not visible
        result.x = projectionPlaneDistance * eyeX / depth * screenScale;
        result.y = projectionPlaneDistance * eyeY / depth * screenScale;
        result.z = -1.0f / depth;   // 1/depth varies linearly across the screen, so it can be interpolated
        result.visible = true;
    }
    return result;
}