// vec3.h : a small 3D vector type and the helpers every other file uses.
#pragma once
#include <cmath>

struct Vec3 { float x, y, z; };

inline Vec3 operator+(Vec3 a, Vec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline Vec3 operator*(Vec3 a, float s) { return { a.x * s, a.y * s, a.z * s }; }
inline Vec3 operator*(float s, Vec3 a) { return a * s; }

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x };
}
inline float length(Vec3 a) { return std::sqrt(dot(a, a)); }

// Returns a unit-length copy. A zero vector is returned unchanged (avoids dividing by 0).
inline Vec3 normalize(Vec3 a) {
    float len = length(a);
    return len > 0.0f ? a * (1.0f / len) : a;
}

inline float clampf(float value, float smallest, float largest) {
    return value < smallest ? smallest : (value > largest ? largest : value);
}

const float DEGREES_TO_RADIANS = 3.14159265f / 180.0f;
