#include "shading.h"

void renderMesh(const Mesh& mesh, const Camera& camera, ShadingMode mode,
    const Light& light, const Material& material, Framebuffer& fb) {

    // Project every vertex once.
    std::vector<ScreenPoint> projected(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); i++)
        projected[i] = camera.project(mesh.vertices[i].position);

    // Gouraud lights the vertices up front (once per vertex, not once per triangle corner).
    std::vector<ColorRGB> vertexColors;
    if (mode == GOURAUD) {
        vertexColors.resize(mesh.vertices.size());
        for (size_t i = 0; i < mesh.vertices.size(); i++)
            vertexColors[i] = calculatePhongLighting(mesh.vertices[i].position, mesh.vertices[i].normal,
                camera.eye, light, material);
    }

    for (const Triangle& t : mesh.triangles) {
        const ScreenPoint& s0 = projected[t.a];
        const ScreenPoint& s1 = projected[t.b];
        const ScreenPoint& s2 = projected[t.c];
        if (!s0.visible || !s1.visible || !s2.visible) continue;   // (no clipping yet)

        RasterPoint r0 = toPixels(fb, s0.x, s0.y, s0.z);
        RasterPoint r1 = toPixels(fb, s1.x, s1.y, s1.z);
        RasterPoint r2 = toPixels(fb, s2.x, s2.y, s2.z);

        const Vertex& v0 = mesh.vertices[t.a];
        const Vertex& v1 = mesh.vertices[t.b];
        const Vertex& v2 = mesh.vertices[t.c];

        if (mode == FLAT) {
            // One lighting evaluation for the whole triangle: face normal, at the centroid.
            Vec3 centroid = (v0.position + v1.position + v2.position) * (1.0f / 3.0f);
            ColorRGB flatColor = calculatePhongLighting(centroid, faceNormal(mesh, t), camera.eye, light, material);
            drawTriangle(fb, r0, r1, r2, [&](float, float, float) { return flatColor; });
        }
        else if (mode == GOURAUD) {
            // Lighting was done at the vertices; blend the three resulting colours.
            ColorRGB c0 = vertexColors[t.a], c1 = vertexColors[t.b], c2 = vertexColors[t.c];
            drawTriangle(fb, r0, r1, r2, [&](float a, float b, float c) {
                return ColorRGB{ a * c0.r + b * c1.r + c * c2.r,
                              a * c0.g + b * c1.g + c * c2.g,
                              a * c0.b + b * c1.b + c * c2.b };
                });
        }
        else {
            // PHONG: blend the normals (and the position), then light every pixel.
            drawTriangle(fb, r0, r1, r2, [&](float a, float b, float c) {
                Vec3 normal = normalize(v0.normal * a + v1.normal * b + v2.normal * c);   // interpolate, then normalize
                Vec3 position = v0.position * a + v1.position * b + v2.position * c;
                return calculatePhongLighting(position, normal, camera.eye, light, material);
                });
        }
    }
}