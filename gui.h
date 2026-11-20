// gui.h : the control panel (Dear ImGui). It only edits the Scene; it never draws the 3D view.
#pragma once
#include "scene.h"

const float PANEL_WIDTH = 360.0f;   // logical pixels, left side of the window

// Call once per frame between ImGui::NewFrame() and ImGui::Render().
// renderMilliseconds: how long the last software render took (shown as a readout).
void drawControlPanel(Scene& scene, float renderMilliseconds);