#include "gui.h"
#include "imgui/imgui.h"

void drawControlPanel(Scene& scene, float renderMilliseconds) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, ImGui::GetIO().DisplaySize.y));
    ImGui::Begin("Controls", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::PushItemWidth(PANEL_WIDTH * 0.52f);   // leaves room for the labels on the right

    // --- Shading --------------------------------------------------------------------------
    ImGui::SeparatorText("Shading");
    int mode = scene.shadingMode;
    ImGui::RadioButton("Flat", &mode, FLAT);       ImGui::SameLine();
    ImGui::RadioButton("Gouraud", &mode, GOURAUD); ImGui::SameLine();
    ImGui::RadioButton("Phong", &mode, PHONG);
    scene.shadingMode = (ShadingMode)mode;
    ImGui::Checkbox("Parallel projection", &scene.camera.useParallelProjection);

    // --- Material properties --------------------------------------------------------------
    // Each control is one property of the lighting equation. Hover a label for what it does.
    ImGui::SeparatorText("Material properties");
    ImGui::TextDisabled("I = ka.La + kd.Ld.(N.L) + ks.Ls.(R.V)^n");

    ImGui::SliderFloat("Ambient (ka)", &scene.material.ambient, 0.0f, 1.0f);
    ImGui::SetItemTooltip("Constant light that reaches every point, even those facing away from the light.\n"
        "Raises the dark side; no shape or highlight.");
    ImGui::SliderFloat("Diffuse (kd)", &scene.material.diffuse, 0.0f, 1.0f);
    ImGui::SetItemTooltip("Scatters light in all directions. Brightest where the surface faces the light (N.L = 1),\n"
        "fading to nothing at 90 degrees. Gives the sphere its 3D shape.");
    ImGui::SliderFloat("Specular (ks)", &scene.material.specular, 0.0f, 1.0f);
    ImGui::SetItemTooltip("Strength of the mirror-like highlight. 0 = no highlight (chalk), high = shiny.");
    ImGui::SliderFloat("Shininess (n)", &scene.material.shininess, 1.0f, 512.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
    ImGui::SetItemTooltip("Exponent on (R.V). Low = wide, soft highlight (rough surface).\n"
        "High = small, sharp highlight (polished surface). Does not change its brightness.");
    ImGui::ColorEdit3("Surface colour", &scene.material.baseColor.r);
    ImGui::SetItemTooltip("Colour of the object itself. Multiplies the ambient and diffuse terms.");
    ImGui::ColorEdit3("Highlight colour", &scene.material.specularColor.r);
    ImGui::SetItemTooltip("Colour of the specular highlight. White for plastic; the same as the surface colour for metals.");

    if (ImGui::Button("Highlight = surface colour")) scene.material.specularColor = scene.material.baseColor;
    ImGui::SameLine();
    if (ImGui::Button("Reset material")) {
        scene.material = defaultMaterial();
        scene.showAmbient = scene.showDiffuse = scene.showSpecular = true;
    }
    if (scene.material.ambient + scene.material.diffuse + scene.material.specular > 1.0f)
        ImGui::TextDisabled("ka + kd + ks > 1: bright areas clip to white");

    ImGui::TextUnformatted("Show terms:");
    ImGui::SameLine(); ImGui::Checkbox("Ambient", &scene.showAmbient);
    ImGui::SameLine(); ImGui::Checkbox("Diffuse", &scene.showDiffuse);
    ImGui::SameLine(); ImGui::Checkbox("Specular", &scene.showSpecular);
    ImGui::SetItemTooltip("Untick terms to see what each one contributes on its own.");

    // --- Light ----------------------------------------------------------------------------
    ImGui::SeparatorText("Light");
    ImGui::SliderFloat("Light x", &scene.light.position.x, -20.0f, 20.0f);
    ImGui::SliderFloat("Light y", &scene.light.position.y, -20.0f, 20.0f);
    ImGui::SliderFloat("Light z", &scene.light.position.z, -20.0f, 20.0f);
    ImGui::ColorEdit3("Light colour", &scene.light.color.x);

    // --- Sphere ---------------------------------------------------------------------------
    ImGui::SeparatorText("Sphere");
    if (ImGui::SliderFloat("Radius", &scene.sphereRadius, 0.5f, 3.5f)) scene.meshDirty = true;
    if (ImGui::SliderInt("Stacks", &scene.stacks, 2, 64)) scene.meshDirty = true;
    if (ImGui::SliderInt("Slices", &scene.slices, 3, 128)) scene.meshDirty = true;
    ImGui::Text("%d vertices, %d triangles", (int)scene.sphere.vertices.size(), (int)scene.sphere.triangles.size());

    // --- Camera ---------------------------------------------------------------------------
    ImGui::SeparatorText("Camera");
    ImGui::SliderFloat("Phi", &scene.camera.phiDegrees, -180.0f, 180.0f, "%.0f deg");
    ImGui::SliderFloat("Theta", &scene.camera.thetaDegrees, -Camera::MAX_THETA_DEGREES, Camera::MAX_THETA_DEGREES, "%.0f deg");
    ImGui::SliderFloat("Distance", &scene.camera.distance, 4.0f, 40.0f);
    ImGui::SliderFloat("Zoom (D)", &scene.camera.projectionPlaneDistance, 0.4f, 40.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

    // --- Performance ----------------------------------------------------------------------
    ImGui::SeparatorText("Performance");
    int size = scene.renderSize;
    if (ImGui::RadioButton("300", size == 300)) size = 300;
    ImGui::SameLine();
    if (ImGui::RadioButton("450", size == 450)) size = 450;
    ImGui::SameLine();
    if (ImGui::RadioButton("600", size == 600)) size = 600;
    ImGui::SameLine();
    ImGui::TextUnformatted("render pixels");
    if (size != scene.renderSize) { scene.renderSize = size; scene.renderSizeChanged = true; }
    ImGui::Text("Software render: %.2f ms", renderMilliseconds);

    // --- Reset / help ---------------------------------------------------------------------
    ImGui::Separator();
    if (ImGui::Button("Reset everything")) {
        Scene fresh;
        scene = fresh;
        scene.meshDirty = true;
        scene.renderSizeChanged = true;
    }
    if (ImGui::CollapsingHeader("Keyboard shortcuts")) {
        ImGui::TextWrapped("1/2/3 shading mode   [ ] shininess\nJ L / I K / U O move light x / y / z\n"
            "Arrows turn camera   W/S move camera\nMouse wheel zoom   P projection   Esc quit");
    }
    ImGui::PopItemWidth();
    ImGui::End();
}