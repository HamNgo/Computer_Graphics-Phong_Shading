// shading.h : the three shading modes. Same mesh, camera, light, material and lighting
// function for all of them; only the place where the lighting is evaluated changes.
#pragma once
#include "mesh.h"
#include "camera.h"
#include "lighting.h"
#include "raster.h"

enum ShadingMode { FLAT, GOURAUD, PHONG };

// Draws the mesh into the framebuffer (the caller clears it first).
void renderMesh(const Mesh& mesh, const Camera& camera, ShadingMode mode,
    const Light& light, const Material& material, Framebuffer& framebuffer);