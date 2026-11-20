// lighting.h : the Phong illumination equation. ONE function, shared by Flat, Gouraud and Phong
// shading. The three modes differ only in WHERE they call it (per triangle, per vertex, per pixel).
//
//   I = ka * La * base                          ambient : light that is "everywhere"
//     + kd * Ld * base * max(0, N.L)            diffuse : brightest when the surface faces the light
//     + ks * Ls * specColor * max(0, R.V)^n     specular: the shiny highlight
//
//   N = unit surface normal             L = unit vector from the surface toward the light
//   V = unit vector toward the camera   R = L reflected about N  =  2(N.L)N - L
//   n = shininess (bigger = smaller, sharper highlight)
//
// All inputs must be in the same space (we use world space).
#pragma once
#include "vec3.h"
#include "color_rgb.h"

struct Material {
    float ambient;      // ka
    float diffuse;      // kd
    float specular;     // ks
    float shininess;    // n
    ColorRGB baseColor;    // colour of the surface itself
    ColorRGB specularColor = { 1.0f, 1.0f, 1.0f };   // colour of the highlight: white for plastic, = baseColor for metal
};

struct Light {
    Vec3 position;      // world space
    Vec3 color;         // used as La, Ld and Ls (r, g, b each 0..1)
};

// Returns the lit colour, with every channel clamped to 0..1.
// The vectors passed in do not have to be unit length; they are normalized here.
ColorRGB calculatePhongLighting(Vec3 position, Vec3 normal, Vec3 cameraPosition,
    const Light& light, const Material& material);