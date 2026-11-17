#include "mesh.h"

Mesh makeSphere(float radius, int stacks, int slices) {
    Mesh mesh;

    // Rows 0..stacks from the top pole (z = +radius) to the bottom pole.
    // Each row has `slices` vertices; the last one connects back to the first, so there is no seam.
    // The two pole rows collapse to a point (all their vertices share one position).
    for (int row = 0; row <= stacks; row++) {
        float polar = 3.14159265f * row / stacks;
        for (int column = 0; column < slices; column++) {
            float azimuth = 2.0f * 3.14159265f * column / slices;
            Vec3 unit = { std::sin(polar) * std::cos(azimuth),
                          std::sin(polar) * std::sin(azimuth),
                          std::cos(polar) };
            mesh.vertices.push_back({ unit * radius, unit });
        }
    }

    auto index = [&](int row, int column) { return row * slices + (column % slices); };

    for (int row = 0; row < stacks; row++) {
        for (int column = 0; column < slices; column++) {
            int a = index(row, column), d = index(row, column + 1);
            int b = index(row + 1, column), c = index(row + 1, column + 1);
            // Skip the triangle that would be degenerate at a pole.
            if (row != 0)          mesh.triangles.push_back({ a, b, d });
            if (row != stacks - 1) mesh.triangles.push_back({ d, b, c });
        }
    }
    return mesh;
}

Vec3 faceNormal(const Mesh& mesh, const Triangle& t) {
    Vec3 p0 = mesh.vertices[t.a].position;
    Vec3 p1 = mesh.vertices[t.b].position;
    Vec3 p2 = mesh.vertices[t.c].position;
    return normalize(cross(p1 - p0, p2 - p0));
}

void computeSmoothNormals(Mesh& mesh) {
    std::vector<Vec3> sums(mesh.vertices.size(), Vec3{ 0, 0, 0 });
    for (const Triangle& t : mesh.triangles) {
        Vec3 p0 = mesh.vertices[t.a].position;
        Vec3 p1 = mesh.vertices[t.b].position;
        Vec3 p2 = mesh.vertices[t.c].position;
        // The cross product's length is proportional to the triangle's area,
        // so adding it unnormalised gives area-weighted averaging for free.
        Vec3 weighted = cross(p1 - p0, p2 - p0);
        sums[t.a] = sums[t.a] + weighted;
        sums[t.b] = sums[t.b] + weighted;
        sums[t.c] = sums[t.c] + weighted;
    }
    for (size_t i = 0; i < mesh.vertices.size(); i++)
        mesh.vertices[i].normal = normalize(sums[i]);
}