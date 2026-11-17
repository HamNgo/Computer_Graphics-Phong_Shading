// mesh.h : triangle mesh data, the sphere generator, and normal calculation.
#pragma once
#include <vector>
#include "vec3.h"

struct Vertex {
    Vec3 position;   // world space
    Vec3 normal;     // unit length
};

struct Triangle { int a, b, c; };   // indices into Mesh::vertices, counter-clockwise seen from outside

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
};

// Z-up UV sphere centred on the origin. Triangles are counter-clockwise seen from outside.
// Vertex normals are exact (position / radius). Higher stacks/slices = smoother sphere.
Mesh makeSphere(float radius, int stacks, int slices);

// Unit normal of one triangle (from its winding order).
Vec3 faceNormal(const Mesh& mesh, const Triangle& triangle);

// Overwrites every vertex normal with the area-weighted average of the faces touching it.
// Not needed for the sphere (its normals are exact), but it is what any other mesh needs,
// and it is a handy cross-check of the sphere's normals.
void computeSmoothNormals(Mesh& mesh);
