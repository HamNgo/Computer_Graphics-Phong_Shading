// raster.h : framebuffer, Z-buffer and the triangle filler.
//
// drawTriangle() finds every pixel a triangle covers, does the depth test, and asks a
// "shader" function what colour that pixel should be. The shader receives the pixel's
// barycentric weights (alpha, beta, gamma), which is exactly what Gouraud and Phong need:
//     Flat    : shader ignores the weights and returns one colour per triangle
//     Gouraud : shader blends three precomputed vertex colours with the weights
//     Phong   : shader blends the three vertex normals, normalizes, then lights the pixel
//
// Header-only because drawTriangle is a template (the shader is passed in as a lambda).
#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include "color_rgb.h"

// A vertex in pixel coordinates (x right, y up, from the bottom-left corner) plus its depth.
struct RasterPoint { float x, y, z; };

struct Framebuffer {
    int width, height;
    std::vector<unsigned char> pixels;   // RGB, 3 bytes per pixel, row 0 = bottom (what glDrawPixels expects)
    std::vector<float> depth;            // smaller = closer

    Framebuffer(int w = 1, int h = 1) : width(w), height(h), pixels(size_t(w)* h * 3), depth(size_t(w)* h) {}

    void clear(ColorRGB background) {
        for (int i = 0; i < width * height; i++) {
            setColor(i, background);
            depth[i] = std::numeric_limits<float>::max();
        }
    }

    void setColor(int pixelIndex, ColorRGB c) {
        pixels[size_t(pixelIndex) * 3 + 0] = toByte(c.r);
        pixels[size_t(pixelIndex) * 3 + 1] = toByte(c.g);
        pixels[size_t(pixelIndex) * 3 + 2] = toByte(c.b);
    }

    static unsigned char toByte(float v) {
        v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
        return (unsigned char)(v * 255.0f + 0.5f);
    }
};

// Converts a screen position in [-1, 1] (as produced by Camera::project) to pixel coordinates.
inline RasterPoint toPixels(const Framebuffer& fb, float screenX, float screenY, float depth) {
    return { (screenX * 0.5f + 0.5f) * fb.width,
             (screenY * 0.5f + 0.5f) * fb.height,
             depth };
}

// Twice the signed area of triangle (a, b, p). Its sign says which side of edge a->b the point p is on.
inline float edgeFunction(float ax, float ay, float bx, float by, float px, float py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

// Shader: ColorRGB shade(float alpha, float beta, float gamma)
//   alpha, beta, gamma are the weights of p0, p1, p2 at this pixel (they sum to 1).
template <typename Shader>
void drawTriangle(Framebuffer& fb, const RasterPoint& p0, const RasterPoint& p1, const RasterPoint& p2,
    Shader shade) {
    float area = edgeFunction(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
    if (area == 0.0f) return;   // edge-on triangle: covers no pixels

    int minX = std::max(0, int(std::floor(std::min({ p0.x, p1.x, p2.x }))));
    int maxX = std::min(fb.width - 1, int(std::ceil(std::max({ p0.x, p1.x, p2.x }))));
    int minY = std::max(0, int(std::floor(std::min({ p0.y, p1.y, p2.y }))));
    int maxY = std::min(fb.height - 1, int(std::ceil(std::max({ p0.y, p1.y, p2.y }))));

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f, py = y + 0.5f;   // sample at the pixel centre

            // Dividing by the (signed) area makes the weights positive inside the triangle
            // whichever way it is wound, and makes them sum to 1.
            float alpha = edgeFunction(p1.x, p1.y, p2.x, p2.y, px, py) / area;
            float beta = edgeFunction(p2.x, p2.y, p0.x, p0.y, px, py) / area;
            float gamma = edgeFunction(p0.x, p0.y, p1.x, p1.y, px, py) / area;
            if (alpha < 0.0f || beta < 0.0f || gamma < 0.0f) continue;   // outside

            float z = alpha * p0.z + beta * p1.z + gamma * p2.z;
            int pixelIndex = y * fb.width + x;
            if (z >= fb.depth[pixelIndex]) continue;   // something closer is already there

            fb.depth[pixelIndex] = z;
            fb.setColor(pixelIndex, shade(alpha, beta, gamma));
        }
    }
}