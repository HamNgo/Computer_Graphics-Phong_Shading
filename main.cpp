// Phong shading project.
// A sphere drawn with our own software rasterizer (framebuffer + Z-buffer), shown with glDrawPixels,
// in three shading modes that share the same mesh, camera, light, material and lighting function.
//
//   1 / 2 / 3     Flat / Gouraud / Phong shading
//   M             cycle the sphere resolution (coarse / medium / fine)
//   [ / ]         halve / double the shininess
//   J / L         move the light along x      I / K  along y      U / O  along z
//   Arrow keys    turn the camera (phi / theta)      W / S  eye closer / farther
//   Mouse wheel   magnify / shrink the view (changes D)
//   P             perspective / parallel             ESC    quit

#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include "vec3.h"
#include "mesh.h"
#include "camera.h"
#include "raster.h"
#include "lighting.h"
#include "shading.h"

const int RENDER_SIZE = 600;   // the framebuffer is RENDER_SIZE x RENDER_SIZE pixels

Camera camera;
Framebuffer framebuffer(RENDER_SIZE, RENDER_SIZE);

// Sphere at three resolutions: lets us compare how mesh density affects each shading mode.
const int RESOLUTION_COUNT = 3;
const int STACKS[RESOLUTION_COUNT] = { 8, 16, 48 };
const int SLICES[RESOLUTION_COUNT] = { 16, 32, 96 };
Mesh spheres[RESOLUTION_COUNT];
int resolutionIndex = 1;

ShadingMode shadingMode = FLAT;
Light light = { { 8.0f, 4.0f, 8.0f }, { 1.0f, 1.0f, 1.0f } };   // fixed in world space
Material material = { 0.15f, 0.7f, 0.6f, 32.0f, { 0.95f, 0.55f, 0.15f } };

const float TURN_SPEED_DEGREES_PER_SECOND = 60.0f;
const float MOVE_SPEED_UNITS_PER_SECOND = 5.0f;
const float LIGHT_SPEED_UNITS_PER_SECOND = 6.0f;
const float MIN_CAMERA_DISTANCE = 4.0f;
const float MAX_CAMERA_DISTANCE = 100.0f;
const float MAX_LIGHT_COORDINATE = 20.0f;
const float MIN_SHININESS = 1.0f;
const float MAX_SHININESS = 512.0f;
const float SCALE_FACTOR_PER_SCROLL_STEP = 1.1f;

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
void presentFramebuffer(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Square viewport, centred; the image is scaled to fit it.
    int side = windowWidth < windowHeight ? windowWidth : windowHeight;
    glViewport((windowWidth - side) / 2, (windowHeight - side) / 2, side, side);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(float(side) / framebuffer.width, float(side) / framebuffer.height);
    glRasterPos2f(-1.0f, -1.0f);   // bottom-left corner of the viewport
    glDrawPixels(framebuffer.width, framebuffer.height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer.pixels.data());
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void showStatusInTitle(GLFWwindow* window) {
    const char* modeNames[] = { "FLAT", "GOURAUD", "PHONG" };
    char title[400];
    std::snprintf(title, sizeof title,
        "Phong project | %s | %s | sphere %dx%d | shininess %.0f | light %.1f %.1f %.1f"
        " | 1/2/3 shading, M mesh, [ ] shininess, IJKLUO light, P projection",
        modeNames[shadingMode], camera.useParallelProjection ? "PARALLEL" : "PERSPECTIVE",
        STACKS[resolutionIndex], SLICES[resolutionIndex], material.shininess,
        light.position.x, light.position.y, light.position.z);
    glfwSetWindowTitle(window, title);
}

void scrollCallback(GLFWwindow* window, double, double scrollY) {
    float factor = std::pow(SCALE_FACTOR_PER_SCROLL_STEP, float(scrollY));
    camera.projectionPlaneDistance = clampf(camera.projectionPlaneDistance * factor,
        Camera::STARTING_PROJECTION_PLANE_DISTANCE * 0.1f,
        Camera::STARTING_PROJECTION_PLANE_DISTANCE * 10.0f);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_P) camera.useParallelProjection = !camera.useParallelProjection;
    if (key == GLFW_KEY_1) shadingMode = FLAT;
    if (key == GLFW_KEY_2) shadingMode = GOURAUD;
    if (key == GLFW_KEY_3) shadingMode = PHONG;
    if (key == GLFW_KEY_M) resolutionIndex = (resolutionIndex + 1) % RESOLUTION_COUNT;
    if (key == GLFW_KEY_LEFT_BRACKET)  material.shininess = clampf(material.shininess * 0.5f, MIN_SHININESS, MAX_SHININESS);
    if (key == GLFW_KEY_RIGHT_BRACKET) material.shininess = clampf(material.shininess * 2.0f, MIN_SHININESS, MAX_SHININESS);
    showStatusInTitle(window);
}

bool isKeyHeld(GLFWwindow* window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

// Arrow keys turn the camera, W / S move it. Returns true if anything changed.
bool moveCamera(GLFWwindow* window, float secondsPassed) {
    float oldPhi = camera.phiDegrees, oldTheta = camera.thetaDegrees, oldDistance = camera.distance;
    float turn = TURN_SPEED_DEGREES_PER_SECOND * secondsPassed;
    float move = MOVE_SPEED_UNITS_PER_SECOND * secondsPassed;

    if (isKeyHeld(window, GLFW_KEY_RIGHT)) camera.phiDegrees += turn;
    if (isKeyHeld(window, GLFW_KEY_LEFT))  camera.phiDegrees -= turn;
    if (isKeyHeld(window, GLFW_KEY_UP))    camera.thetaDegrees += turn;
    if (isKeyHeld(window, GLFW_KEY_DOWN))  camera.thetaDegrees -= turn;
    if (isKeyHeld(window, GLFW_KEY_W)) camera.distance -= move;
    if (isKeyHeld(window, GLFW_KEY_S)) camera.distance += move;

    if (camera.phiDegrees > 180.0f)  camera.phiDegrees -= 360.0f;    // phi wraps around
    if (camera.phiDegrees < -180.0f) camera.phiDegrees += 360.0f;
    camera.thetaDegrees = clampf(camera.thetaDegrees, -Camera::MAX_THETA_DEGREES, Camera::MAX_THETA_DEGREES);
    camera.distance = clampf(camera.distance, MIN_CAMERA_DISTANCE, MAX_CAMERA_DISTANCE);

    return camera.phiDegrees != oldPhi || camera.thetaDegrees != oldTheta || camera.distance != oldDistance;
}

// J / L, I / K and U / O move the light. Returns true if the light moved.
bool moveLight(GLFWwindow* window, float secondsPassed) {
    float move = LIGHT_SPEED_UNITS_PER_SECOND * secondsPassed;
    Vec3 old = light.position;

    if (isKeyHeld(window, GLFW_KEY_L)) light.position.x += move;
    if (isKeyHeld(window, GLFW_KEY_J)) light.position.x -= move;
    if (isKeyHeld(window, GLFW_KEY_I)) light.position.y += move;
    if (isKeyHeld(window, GLFW_KEY_K)) light.position.y -= move;
    if (isKeyHeld(window, GLFW_KEY_U)) light.position.z += move;
    if (isKeyHeld(window, GLFW_KEY_O)) light.position.z -= move;

    light.position.x = clampf(light.position.x, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);
    light.position.y = clampf(light.position.y, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);
    light.position.z = clampf(light.position.z, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);

    return light.position.x != old.x || light.position.y != old.y || light.position.z != old.z;
}

// ---------------------------------------------------------------------------
int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(700, 700, "Phong project", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);

    for (int i = 0; i < RESOLUTION_COUNT; i++)
        spheres[i] = makeSphere(3.0f, STACKS[i], SLICES[i]);
    camera.update();
    showStatusInTitle(window);

    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float secondsPassed = float(currentTime - previousTime);
        previousTime = currentTime;

        bool cameraMoved = moveCamera(window, secondsPassed);
        bool lightMoved = moveLight(window, secondsPassed);
        if (cameraMoved || lightMoved) showStatusInTitle(window);
        camera.update();

        framebuffer.clear({ 0.1f, 0.1f, 0.12f });
        renderMesh(spheres[resolutionIndex], camera, shadingMode, light, material, framebuffer);
        presentFramebuffer(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}