// scene.h : everything the user can change while the program runs, in one place.
// main.cpp renders it; gui.cpp edits it.
#pragma once
#include "mesh.h"
#include "camera.h"
#include "lighting.h"
#include "shading.h"

// The material the program starts with (and what "Reset material" returns to).
inline Material defaultMaterial() {
    return { 0.15f, 0.7f, 0.6f, 32.0f, { 0.95f, 0.55f, 0.15f }, { 1.0f, 1.0f, 1.0f } };
}

struct Scene {
    Camera camera;
    Light light = { { 8.0f, 4.0f, 8.0f }, { 1.0f, 1.0f, 1.0f } };   // fixed in world space
    Material material = defaultMaterial();
    // Switch a term of the lighting equation off to see what the others contribute on their own.
    bool showAmbient = true;
    bool showDiffuse = true;
    bool showSpecular = true;
    ShadingMode shadingMode = PHONG;

    float sphereRadius = 3.0f;
    int stacks = 16;
    int slices = 32;
    bool meshDirty = true;            // set when radius / stacks / slices change: the sphere is rebuilt
    Mesh sphere;

    int renderSize = 600;             // framebuffer is renderSize x renderSize pixels
    bool renderSizeChanged = false;
};