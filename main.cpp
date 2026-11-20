// Phong shading project.
// A sphere drawn with our own software rasterizer (framebuffer + Z-buffer), shown with glDrawPixels,
// in three shading modes that share the same mesh, camera, light, material and lighting function.
// A Dear ImGui panel on the left edits everything live (gui.cpp). The keyboard shortcuts still work:
//
//   1 / 2 / 3     Flat / Gouraud / Phong shading        [ / ]   halve / double the shininess
//   M             cycle the sphere resolution            P       perspective / parallel
//   J / L         move the light along x      I / K  along y      U / O  along z
//   Arrow keys    turn the camera (phi / theta)      W / S  eye closer / farther
//   Mouse wheel   magnify / shrink the view (changes D)      ESC  quit

#include <GLFW/glfw3.h>
#include <cstdio>
#include <vector>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"
#include "vec3.h"
#include "mesh.h"
#include "camera.h"
#include "raster.h"
#include "lighting.h"
#include "shading.h"
#include "scene.h"
#include "gui.h"

Scene scene;
Framebuffer framebuffer;

// M cycles through these sphere resolutions (the GUI sliders can set any value).
const int RESOLUTION_COUNT = 3;
const int STACKS[RESOLUTION_COUNT] = { 8, 16, 48 };
const int SLICES[RESOLUTION_COUNT] = { 16, 32, 96 };

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
// Shows the framebuffer in the largest square to the right of the control panel.
void presentFramebuffer(GLFWwindow* window) {
    int windowWidth, windowHeight, pixelWidth, pixelHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &pixelWidth, &pixelHeight);

    glViewport(0, 0, pixelWidth, pixelHeight);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float pixelsPerPoint = windowWidth > 0 ? float(pixelWidth) / windowWidth : 1.0f;   // > 1 on high-DPI screens
    int panelPixels = int(PANEL_WIDTH * pixelsPerPoint);
    int areaWidth = pixelWidth - panelPixels;
    int side = areaWidth < pixelHeight ? areaWidth : pixelHeight;
    if (side < 1) return;   // window too small to show the image

    glViewport(panelPixels + (areaWidth - side) / 2, (pixelHeight - side) / 2, side, side);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(float(side) / framebuffer.width, float(side) / framebuffer.height);
    glRasterPos2f(-1.0f, -1.0f);   // bottom-left corner of the viewport
    glDrawPixels(framebuffer.width, framebuffer.height, GL_RGB, GL_UNSIGNED_BYTE, framebuffer.pixels.data());
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void scrollCallback(GLFWwindow*, double, double scrollY) {
    if (ImGui::GetIO().WantCaptureMouse) return;   // the wheel is scrolling the panel
    float factor = std::pow(SCALE_FACTOR_PER_SCROLL_STEP, float(scrollY));
    scene.camera.projectionPlaneDistance = clampf(scene.camera.projectionPlaneDistance * factor,
        Camera::STARTING_PROJECTION_PLANE_DISTANCE * 0.1f,
        Camera::STARTING_PROJECTION_PLANE_DISTANCE * 10.0f);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (ImGui::GetIO().WantCaptureKeyboard) return;   // typing into a panel widget
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_P) scene.camera.useParallelProjection = !scene.camera.useParallelProjection;
    if (key == GLFW_KEY_1) scene.shadingMode = FLAT;
    if (key == GLFW_KEY_2) scene.shadingMode = GOURAUD;
    if (key == GLFW_KEY_3) scene.shadingMode = PHONG;
    if (key == GLFW_KEY_M) {
        int next = 0;   // the first preset above the current resolution, wrapping around
        for (int i = 0; i < RESOLUTION_COUNT; i++)
            if (STACKS[i] * SLICES[i] > scene.stacks * scene.slices) { next = i; break; }
        scene.stacks = STACKS[next];
        scene.slices = SLICES[next];
        scene.meshDirty = true;
    }
    if (key == GLFW_KEY_LEFT_BRACKET)
        scene.material.shininess = clampf(scene.material.shininess * 0.5f, MIN_SHININESS, MAX_SHININESS);
    if (key == GLFW_KEY_RIGHT_BRACKET)
        scene.material.shininess = clampf(scene.material.shininess * 2.0f, MIN_SHININESS, MAX_SHININESS);
}

bool isKeyHeld(GLFWwindow* window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

// Arrow keys turn the camera, W / S move it.
void moveCamera(GLFWwindow* window, float secondsPassed) {
    Camera& camera = scene.camera;
    float turn = TURN_SPEED_DEGREES_PER_SECOND * secondsPassed;
    float move = MOVE_SPEED_UNITS_PER_SECOND * secondsPassed;

    if (isKeyHeld(window, GLFW_KEY_RIGHT)) camera.phiDegrees += turn;
    if (isKeyHeld(window, GLFW_KEY_LEFT))  camera.phiDegrees -= turn;
    if (isKeyHeld(window, GLFW_KEY_UP))    camera.thetaDegrees += turn;
    if (isKeyHeld(window, GLFW_KEY_DOWN))  camera.thetaDegrees -= turn;
    if (isKeyHeld(window, GLFW_KEY_W)) camera.distance -= move;
    if (isKeyHeld(window, GLFW_KEY_S)) camera.distance += move;
}

// J / L, I / K and U / O move the light.
void moveLight(GLFWwindow* window, float secondsPassed) {
    float move = LIGHT_SPEED_UNITS_PER_SECOND * secondsPassed;
    Vec3& p = scene.light.position;

    if (isKeyHeld(window, GLFW_KEY_L)) p.x += move;
    if (isKeyHeld(window, GLFW_KEY_J)) p.x -= move;
    if (isKeyHeld(window, GLFW_KEY_I)) p.y += move;
    if (isKeyHeld(window, GLFW_KEY_K)) p.y -= move;
    if (isKeyHeld(window, GLFW_KEY_U)) p.z += move;
    if (isKeyHeld(window, GLFW_KEY_O)) p.z -= move;
}

// Keeps values inside the ranges the renderer and the panel sliders assume (keys and the GUI both edit them).
void limitSceneValues() {
    Camera& camera = scene.camera;
    if (camera.phiDegrees > 180.0f)  camera.phiDegrees -= 360.0f;    // phi wraps around
    if (camera.phiDegrees < -180.0f) camera.phiDegrees += 360.0f;
    camera.thetaDegrees = clampf(camera.thetaDegrees, -Camera::MAX_THETA_DEGREES, Camera::MAX_THETA_DEGREES);
    camera.distance = clampf(camera.distance, MIN_CAMERA_DISTANCE, MAX_CAMERA_DISTANCE);
    Vec3& p = scene.light.position;
    p.x = clampf(p.x, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);
    p.y = clampf(p.y, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);
    p.z = clampf(p.z, -MAX_LIGHT_COORDINATE, MAX_LIGHT_COORDINATE);
}

// ---------------------------------------------------------------------------
int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1100, 780, "Phong project", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Our callbacks first, then ImGui: ImGui installs its own and calls ours afterwards.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;   // don't write imgui.ini next to the program
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    float renderMilliseconds = 0.0f;
    double previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float secondsPassed = float(currentTime - previousTime);
        previousTime = currentTime;

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            moveCamera(window, secondsPassed);
            moveLight(window, secondsPassed);
        }
        drawControlPanel(scene, renderMilliseconds);
        ImGui::Render();

        // Apply whatever the keys and the panel changed.
        limitSceneValues();
        scene.camera.update();
        if (scene.meshDirty) {
            scene.sphere = makeSphere(scene.sphereRadius, scene.stacks, scene.slices);
            scene.meshDirty = false;
        }
        if (scene.renderSizeChanged) {
            framebuffer = Framebuffer(scene.renderSize, scene.renderSize);
            scene.renderSizeChanged = false;
        }
        if (framebuffer.width != scene.renderSize) framebuffer = Framebuffer(scene.renderSize, scene.renderSize);

        // The material used this frame: a term that is switched off in the panel simply gets coefficient 0.
        Material material = scene.material;
        if (!scene.showAmbient)  material.ambient = 0.0f;
        if (!scene.showDiffuse)  material.diffuse = 0.0f;
        if (!scene.showSpecular) material.specular = 0.0f;

        double renderStart = glfwGetTime();
        framebuffer.clear({ 0.1f, 0.1f, 0.12f });
        renderMesh(scene.sphere, scene.camera, scene.shadingMode, scene.light, material, framebuffer);
        float thisFrame = float((glfwGetTime() - renderStart) * 1000.0);
        renderMilliseconds = renderMilliseconds == 0.0f ? thisFrame : 0.9f * renderMilliseconds + 0.1f * thisFrame;

        presentFramebuffer(window);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}