# Phong Shading Project

A C++ / GLFW program that renders a sphere with **Flat**, **Gouraud** and **Phong** shading, using its own software triangle rasterizer, so the three techniques can be compared on exactly the same mesh, camera, light and material. A control panel lets you change every material property live and switch individual terms of the lighting equation on and off.

## Contents

1. [Current status](#current-status)
2. [Run it](#run-it)
3. [Build it (Windows, Visual Studio)](#build-it-windows-visual-studio)
4. [Controls](#controls)
5. [Project structure](#project-structure)
6. [How the rendering works](#how-the-rendering-works)
7. [Implementation notes and known limitations](#implementation-notes-and-known-limitations)
8. [Third-party code](#third-party-code)
9. [Project roadmap](#project-roadmap)

---

## Current status

**Working**

- Software rasterizer: triangle coverage with barycentric weights, Z-buffer, pixel writes. The finished image is displayed with `glDrawPixels`.
- UV sphere mesh (adjustable radius, stacks, slices) with face normals and smooth vertex normals.
- One shared Phong illumination function (ambient + diffuse + specular).
- Flat, Gouraud and Phong shading modes, selectable at run time.
- Orbit camera with perspective and parallel projection.
- Control panel (Dear ImGui): shading mode, material properties, light, sphere, camera, render resolution, and a render-time readout.
- Per-term switches (ambient / diffuse / specular) to show what each term contributes on its own.

**Not done yet**

- Key to save screenshots for the report.
- Fixed test configurations for the comparison (shininess values, light positions).
- Results table filled in from real observations and timings.
- Report and slides.

**Decisions made so far**

| Decision | Reason |
|---|---|
| Our own CPU rasterizer instead of the old `GL_POLYGON` drawing | Phong needs per-pixel normals and barycentric weights; the fixed-function OpenGL pipeline hides both. |
| Sphere as the test object | Exact normals, and highlights are easy to see. |
| World space for all lighting vectors | One consistent coordinate space for position, normal, light and camera. |
| Dear ImGui for the control panel | The roadmap allowed extra tooling once the core was complete. It is the only third-party library besides GLFW. |

---

## Run it

If you have the prebuilt `phong.exe`, double-click it. It is a single file with no DLLs and no installer. Windows SmartScreen or an antivirus may warn about an unsigned program; that is expected for a locally built executable.

Otherwise, build it yourself (next section).

---

## Build it (Windows, Visual Studio)

### What you need

- **Visual Studio 2019 or 2022** with the *Desktop development with C++* workload.
- **GLFW 3.4, pre-compiled Windows binaries** from <https://www.glfw.org/download.html> (the "64-bit Windows binaries" zip). Unzip it somewhere permanent. It contains `include\` and several `lib-vc20xx\` folders.
- **Dear ImGui 1.91.9**, already included in this project's `imgui\` folder. (If the folder is missing, see [Getting ImGui](#getting-imgui) below.)

### Steps

1. **Create a project** (Console App, C++) or open your existing one. Use a project folder you control.
2. **Copy the source files** into the project folder, with the `imgui\` folder placed **next to `main.cpp`**. `main.cpp` includes `"imgui/imgui.h"`, so the location matters.
3. **Add the files to the project** (Solution Explorer, Add, Existing Item):
   - `main.cpp`, `gui.cpp`, `mesh.cpp`, `camera.cpp`, `lighting.cpp`, `shading.cpp`
   - from `imgui\`: `imgui.cpp`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `imgui_impl_glfw.cpp`, `imgui_impl_opengl2.cpp`

   Header files (`.h`) only need to be in the folders; they don't have to be added to the project.
4. **Point the project at GLFW** (Project Properties, with *Configuration: All Configurations* and *Platform: x64*):
   - C/C++, General, *Additional Include Directories*: `<glfw folder>\include`
   - Linker, General, *Additional Library Directories*: `<glfw folder>\lib-vc2022` (use `lib-vc2019` if you build with VS 2019)
   - Linker, Input, *Additional Dependencies*: add `glfw3.lib;opengl32.lib;`
5. **Set the language standard**: C/C++, Language, *C++ Language Standard*: **ISO C++17**.
6. **Select `Release` and `x64`** in the toolbar. Do not use Debug (see the note below).
7. **Build and run** with Ctrl+F5.

No other libraries are needed. ImGui links `user32`, `shell32` and `imm32` itself through `#pragma comment`.

> **Use Release.** This renderer shades pixels on the CPU. In a Debug build the compiler does not inline the many small vector functions, which makes it many times slower and the window lag badly.

### Alternative: MinGW-w64 (command line)

This is how `phong.exe` was produced (cross-compiled). From the project folder, with `GLFW` pointing at the unzipped GLFW folder:

```text
g++ -std=c++17 -O2 main.cpp gui.cpp mesh.cpp camera.cpp lighting.cpp shading.cpp ^
    imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp ^
    imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl2.cpp ^
    -I%GLFW%/include -L%GLFW%/lib-mingw-w64 -lglfw3 -lgdi32 -lopengl32 ^
    -static -static-libgcc -static-libstdc++ -o phong.exe
```

### Getting ImGui

Only needed if the `imgui\` folder is missing. Download **version 1.91.9** (use this version; newer ones changed some functions we call) from <https://github.com/ocornut/imgui/releases/tag/v1.91.9> ("Source code (zip)"). Create `imgui\` next to `main.cpp` and copy these files into it, flat, with no subfolders:

- from the top level of the download: `imgui.cpp`, `imgui.h`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `imgui_internal.h`, `imconfig.h`, `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`, `LICENSE.txt`
- from its `backends\` folder: `imgui_impl_glfw.cpp`, `imgui_impl_glfw.h`, `imgui_impl_opengl2.cpp`, `imgui_impl_opengl2.h`

### Troubleshooting

| Problem | Likely cause and fix |
|---|---|
| `cannot open include file 'GLFW/glfw3.h'` | GLFW's `include` folder is not in *Additional Include Directories*. |
| `unresolved external symbol glfw...` | GLFW's `lib-vc20xx` folder is not in *Additional Library Directories*, or `glfw3.lib` is missing from *Additional Dependencies*. |
| `unresolved external symbol gl...` | `opengl32.lib` is missing from *Additional Dependencies*. |
| `cannot open include file 'imgui/imgui.h'` | The `imgui` folder is not next to `main.cpp`. |
| Unresolved externals for `ImGui::...` | The six `imgui\*.cpp` files were not added to the project. |
| Errors about `Color` being redefined or a function overloaded by return type | An old copy of a project file is still in the folder (for example an old `color.h`; the type is now `ColorRGB` in `color_rgb.h`). Remove the old files. |
| Window opens but moves in jerks | You are running a Debug build. Switch to Release, x64. |

---

## Controls

### Control panel (left side of the window)

| Section | What it does |
|---|---|
| **Shading** | Flat / Gouraud / Phong; perspective or parallel projection. |
| **Material properties** | Ambient (ka), Diffuse (kd), Specular (ks), Shininess (n), surface colour, highlight colour. Hover a control for an explanation. *Show terms* switches the ambient, diffuse and specular terms on or off. *Reset material* restores the defaults. |
| **Light** | Position (x, y, z) and colour. |
| **Sphere** | Radius, stacks and slices (the mesh is rebuilt live) and the vertex/triangle count. |
| **Camera** | Phi, theta, distance and zoom (D). |
| **Performance** | Render resolution (300 / 450 / 600 pixels) and the software render time in milliseconds. |

### Keyboard and mouse

| Input | Action |
|---|---|
| `1` / `2` / `3` | Flat / Gouraud / Phong |
| `M` | Cycle the sphere resolution (coarse / medium / fine) |
| `[` / `]` | Halve / double the shininess |
| `J` `L` / `I` `K` / `U` `O` | Move the light along x / y / z |
| Arrow keys | Turn the camera (phi, theta) |
| `W` / `S` | Move the eye closer / farther |
| Mouse wheel | Magnify / shrink the view (changes D) |
| `P` | Perspective / parallel projection |
| `Esc` | Quit |

The keys are ignored while you are typing in a panel widget, and the mouse wheel scrolls the panel while the pointer is over it.

---

## Project structure

| File | Purpose |
|---|---|
| `main.cpp` | Window, main loop, keyboard and mouse input, showing the framebuffer. |
| `scene.h` | Everything that can change at run time (camera, light, material, mode, sphere settings). |
| `gui.h` / `gui.cpp` | The control panel (Dear ImGui). It only edits the scene. |
| `vec3.h` | `Vec3` and vector helpers. |
| `color_rgb.h` | `ColorRGB`, an RGB colour with channels 0 to 1. |
| `mesh.h` / `mesh.cpp` | Vertex, triangle and mesh types; the sphere generator; face normals; smooth vertex normals. |
| `camera.h` / `camera.cpp` | Orbit camera and the projection from world space to screen space. |
| `raster.h` | Framebuffer, Z-buffer and `drawTriangle` (header-only template). |
| `lighting.h` / `lighting.cpp` | `Material`, `Light` and `calculatePhongLighting`, the one lighting function shared by all modes. |
| `shading.h` / `shading.cpp` | `renderMesh` and the three modes: `FLAT`, `GOURAUD`, `PHONG`. |
| `imgui\` | Dear ImGui 1.91.9 (third-party). |

---

## How the rendering works

Each frame goes through these stages:

1. **Geometry.** The sphere is a triangle mesh; every vertex has a position and a unit normal.
2. **Projection** (`camera.cpp`). Each vertex position is projected to the screen, with a depth value for the Z-buffer.
3. **Rasterization** (`raster.h`). For each triangle, every covered pixel gets barycentric weights (alpha, beta, gamma), a depth test, and a colour from a small "shader" function.
4. **Shading** (`shading.cpp`). The same lighting function (`calculatePhongLighting`) is used by all three modes. Only **where it is evaluated** changes:

| Mode | Where lighting is evaluated | What is interpolated |
|---|---|---|
| Flat | once per triangle, at its centroid, with the face normal | nothing: one colour fills the triangle |
| Gouraud | at the three vertices | the resulting colours |
| Phong | at every covered pixel | the vertex normals (then normalized) and the position |

The lighting equation:

```text
I = ka * La * base + kd * Ld * base * max(0, N . L) + ks * Ls * specColor * max(0, R . V)^n
```

where `N` is the unit normal, `L` the unit vector to the light, `V` the unit vector to the camera, `R = 2(N . L)N - L` is `L` reflected about `N`, and `n` is the shininess. Final channel values are clamped to the range 0 to 1.

---

## Implementation notes and known limitations

- **Screen-space interpolation.** Barycentric weights are measured on the screen and are used directly for normals, positions and colours. Under perspective projection this is an approximation (perspective-correct interpolation is the roadmap's optional improvement and is not implemented).
- **View vector under parallel projection.** The lighting function is always given the camera's eye position, so `V` varies per point even in parallel mode.
- **No near-plane clipping.** A triangle with any vertex behind the eye is skipped. The minimum camera distance prevents this in normal use.
- **No backface culling.** Back-facing triangles are drawn and lit too (the Z-buffer hides them).
- **Light is fixed in world space.**
- **Specular is skipped when `N . L <= 0`.** The roadmap's formula has no such check; it only matters where the light is behind the surface.
- **Material extras.** The material has a separate highlight colour in addition to the roadmap's ka, kd, ks and n, and the *Show terms* switches work by setting a coefficient to 0 for that frame.
- **Performance.** All shading is on the CPU, so the cost grows with the number of covered pixels. Phong is roughly twice as expensive as Gouraud in this renderer because it evaluates the lighting at every pixel; the panel shows the render time on your machine.

---

## Third-party code

- **[Dear ImGui](https://github.com/ocornut/imgui)** 1.91.9, MIT licence (`imgui\LICENSE.txt`).
- **[GLFW](https://www.glfw.org/)** 3.4, zlib licence (windowing and input; not included here).

---

## Project roadmap

| | |
|---|---|
| **Team size** | 3 people |
| **Environment** | C++ with the existing GLFW project |
| **Objective** | Implement Phong shading and compare it fairly with Flat and Gouraud shading |
| **Time limit** | 2 weeks (14 days) |

---

### 1. Project scope and decisions

#### Main deliverable

A working C++ application that renders the same 3D object under three selectable shading modes:

1. **Flat shading**: one lighting value per triangle, computed from its face normal.
2. **Gouraud shading**: lighting computed at each vertex, with the resulting vertex colors interpolated across the triangle.
3. **Phong shading**: vertex normals interpolated across the triangle, with lighting computed per pixel.

Phong shading is the main subject of the project. Flat and Gouraud shading are comparison baselines.

#### Keep the existing setup

- Continue with the current C++ / GLFW renderer. Do not switch to another graphics library during this project.
- Reuse the existing camera, projection, vector math, window, and depth-buffer code wherever possible.
- Do not add texture mapping, shadows, PBR, complex model loading, or a sophisticated GUI unless the core project is already complete.

#### Important feasibility check

Before implementation, verify whether the current renderer can already:

- draw filled triangles (not only points or lines);
- determine which pixels lie inside a triangle;
- perform depth testing with a Z-buffer;
- write a color to an individual pixel.

If these features already exist, reuse them. If filled-triangle rasterization is missing, make a minimal software triangle rasterizer the first priority. Do not build a general-purpose graphics engine.

---

### 2. Team responsibilities

Assign one person to each area. Everyone helps with integration and testing near the end.

| Member | Main responsibility | Concrete deliverables |
|---|---|---|
| **Member 1: Geometry and rasterization** | Mesh data, face normals, vertex normals, triangle coverage, depth testing | A sphere/mesh rendered as filled triangles; correct face and vertex normals; per-pixel access to triangle data |
| **Member 2: Shading algorithms** | Shared Phong illumination equation; Flat, Gouraud, and Phong modes | One reusable lighting function and three working shading modes |
| **Member 3: Integration and evaluation** | Connecting modes to the application; controls; test scenes; screenshots; report and slides | Shading-mode selection, consistent comparison scenes, recorded results, documentation |

**Member 3 is not only a report writer.** The comparison modes and test setup are technical project work.

#### Shared interface agreement

Agree on common data structures and coordinate conventions before coding. Adapt these examples to the project's existing types instead of rewriting working code.

```cpp
struct Vertex {
    Vec3 position;
    Vec3 normal;
};

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

struct Light {
    Vec3 position;
    Vec3 color;
};

enum ShadingMode {
    FLAT,
    GOURAUD,
    PHONG
};
```

Agree on one lighting function:

```cpp
Color calculatePhongLighting(
    Vec3 position,
    Vec3 normal,
    Vec3 cameraPosition,
    Light light,
    Material material
);
```

The function should receive vectors in a consistent coordinate space. For the first implementation, world space is a reasonable choice: positions, normals, light position, and camera position must all use world space. If the existing renderer uses another consistent space, document and keep that convention.

---

### 3. Fourteen-day schedule

#### Days 1-2: Inspect, agree, and prepare

**All members**
- Build and run the current project before changing anything.
- Identify which parts already work: camera, projection, mesh, filled triangles, pixel writes, and Z-buffer.
- Agree on coordinate conventions, data structures, and function signatures.
- Choose one test object: a sphere is recommended.
- Create a shared branch or agree on how code changes will be merged.

**Member 1**
- Inspect the mesh representation and triangle rasterization.
- Check whether face normals and vertex normals can be stored.

**Member 2**
- Review the Phong illumination equation and prepare pseudocode.
- Confirm the lighting function's inputs and coordinate space.

**Member 3**
- Define the three comparison modes and prepare a simple test checklist.
- Prepare the README/report outline and screenshot naming convention.

**Milestone:** Everyone can build the same starting project, and the team knows exactly what is missing.

#### Days 3-4: Geometry and baseline rendering

**Member 1**
- Ensure a sphere or simple mesh is represented as triangles.
- Calculate each triangle's face normal.
- Generate smooth vertex normals by averaging adjacent face normals (area-weighted averaging is preferable if straightforward).
- Verify normal directions and normalize the results.

**Member 2**
- Implement the vector operations needed by the lighting equation.
- Implement and test the shared ambient, diffuse, and specular lighting function using known vectors.
- Clamp final color values to the valid display range.

**Member 3**
- Prepare a fixed camera, light, and material configuration.
- Decide how users will switch between the three modes (a simple keyboard key is enough).

**Milestone:** The object renders reliably, and the team can inspect valid face and vertex normals.

#### Days 5-6: Flat and Gouraud shading

**Member 1**
- Finish or repair filled-triangle rasterization and Z-buffer integration if needed.
- Expose the triangle's vertices and pixel coverage to the shading code.

**Member 2**
- Flat shading: evaluate lighting once per triangle using its face normal.
- Gouraud shading: evaluate lighting at the three vertices and interpolate the resulting colors across the triangle.

**Member 3**
- Connect a simple mode selector.
- Verify that Flat and Gouraud modes use the same mesh, camera, light, and material.

**Milestone:** Flat and Gouraud modes work. Keep this as a stable checkpoint before adding Phong shading.

#### Days 7-9: Phong shading

**Member 1**
- Provide the three vertex normals and barycentric weights for each covered pixel.
- Ensure the interpolated normal is normalized before it is used.

**Member 2**
- Phong shading: interpolate the vertex normals across each triangle, normalize the interpolated normal, then evaluate lighting per pixel.
- Use the same lighting function already used by Flat and Gouraud modes.
- Check the specular term carefully; normalize the light, view, and normal vectors.

**Member 3**
- Integrate the Phong mode into the selector.
- Prepare a test with a small, bright specular highlight so the difference between Gouraud and Phong is visible.

**Milestone:** All three shading modes render the same object.

#### Days 10-11: Debugging and fair comparison

**All members**
- Fix visible artifacts, incorrect normal directions, inconsistent coordinate spaces, and depth errors.
- Compare modes with identical scene settings.
- Test at least two shininess values and two light positions.
- Record any limitations, such as perspective interpolation or performance.

**Member 1**
- Verify mesh normals and Z-buffer behavior.

**Member 2**
- Verify lighting behavior and compare the highlight in Gouraud versus Phong.

**Member 3**
- Capture comparison screenshots and record observations.

**Milestone:** The three modes are stable enough to demonstrate.

#### Days 12-13: Report and presentation

**Member 1**
- Document mesh representation, face normals, vertex normals, and rasterization.

**Member 2**
- Document the shading equations, interpolation, and implementation differences.

**Member 3**
- Assemble the report, screenshots, comparison table, and presentation.
- Include a short explanation of why Phong shading can represent small specular highlights better than Gouraud shading.

**All members**
- Rehearse the demonstration.
- Test the project from a clean build.
- Keep a known-working backup.

**Milestone:** Report, slides, and demo are ready.

#### Day 14: Final verification and demonstration

- Build and run the final version.
- Demonstrate Flat, Gouraud, and Phong shading on the same object.
- Change the light position and shininess.
- Explain the visual and computational differences.
- Keep a backup copy of the source code and final presentation.

**Final milestone:** A reproducible demonstration and complete project submission.

---

### 4. Implementation reference

#### Shared Phong illumination

All three shading techniques use the same Phong illumination equation:

```text
I = ka * La
  + kd * Ld * max(0, N · L)
  + ks * Ls * max(0, R · V)^n
```

Where:

- `N`: normalized surface normal.
- `L`: normalized direction from the surface toward the light.
- `V`: normalized direction from the surface toward the camera.
- `R`: normalized reflection direction of the incoming light.
- `ka`, `kd`, `ks`: material ambient, diffuse, and specular coefficients.
- `La`, `Ld`, `Ls`: ambient, diffuse, and specular light intensities.
- `n`: shininess exponent.

The main difference between the three modes is **where the lighting equation is evaluated**.

#### Flat shading

1. Calculate one face normal for the triangle.
2. Choose a representative position for the triangle (for example, its centroid).
3. Evaluate lighting once using that face normal.
4. Fill the triangle with the resulting color.

#### Gouraud shading

1. Obtain a normal at each vertex.
2. Evaluate lighting separately at the three vertices.
3. Interpolate the three resulting vertex colors across the triangle.
4. Write the interpolated color to each covered pixel.

#### Phong shading

1. Obtain a normal at each vertex.
2. For each covered pixel, interpolate the three vertex normals using barycentric weights:

   ```text
   N = alpha * N1 + beta * N2 + gamma * N3
   where alpha + beta + gamma = 1
   ```

3. Normalize the interpolated normal:

   ```text
   N_normalized = N / length(N)
   ```

4. Evaluate the Phong illumination equation at that pixel using the interpolated normal and the pixel's position.

**Important:** Phong shading does not mean interpolating the final colors. It means interpolating normals first, then calculating lighting separately at each pixel.

#### Practical implementation note

For a first software-renderer implementation, screen-space barycentric interpolation is a practical starting point. Under perspective projection, perspective-correct interpolation is more accurate, but it can be treated as an optional improvement if time is limited.

---

### 5. Comparison and evaluation plan

Use the **same object, camera, light, material, and display settings** for each shading mode. Change only the shading mode.

| Test | Setup | What to observe |
|---|---|---|
| Basic appearance | Same sphere and neutral light | Faceted Flat shading versus smoother Gouraud and Phong shading |
| Specular highlight | Small, bright highlight | Gouraud may miss a highlight between vertices; Phong evaluates lighting across the surface |
| Shininess | Test a low and a high exponent | Highlight size and intensity |
| Light movement | Move the light around the sphere | Whether illumination changes smoothly and consistently |
| Mesh resolution | Compare low- and higher-polygon spheres | How mesh density affects visible faceting and smoothness |

#### Suggested results table

Fill this in after running the tests; do not invent results.

| Observation | Flat | Gouraud | Phong |
|---|---|---|---|
| Smooth appearance | | | |
| Small specular highlights | | | |
| Visible polygon boundaries | | | |
| Relative computation cost | | | |

#### Expected general findings

- Flat shading visibly emphasizes individual polygon faces.
- Gouraud shading smooths color transitions but can miss small specular highlights.
- Phong shading usually produces smoother and more accurate specular highlights because lighting is evaluated per pixel.
- Phong shading generally requires more lighting calculations than Gouraud shading.

If performance is measured, use the same resolution, mesh, and hardware conditions, and report the measurement method.

---

### 6. Minimum viable submission

If time becomes tight, prioritize in this order:

1. A working filled-triangle renderer with depth testing.
2. Correct face and vertex normals.
3. A working shared Phong illumination function.
4. Flat, Gouraud, and Phong shading modes.
5. A fair side-by-side comparison.
6. Report and presentation polish.

Do **not** sacrifice a working Phong implementation to add optional features.

#### Features to skip unless everything above is complete

- Texture mapping
- Shadows and shadow mapping
- Physically based rendering
- Complex imported models
- Advanced GUI controls
- Multiple light types
- Additional graphics libraries

---

### 7. Final team checklist

#### Core implementation

- [ ] Existing C++/GLFW project builds and runs.
- [ ] A filled 3D mesh renders with depth testing.
- [ ] Face normals are correct.
- [ ] Smooth vertex normals are correct.
- [ ] Flat shading works.
- [ ] Gouraud shading works.
- [ ] Phong shading interpolates and normalizes normals per pixel.
- [ ] All modes use consistent lighting and material settings.

#### Demonstration

- [ ] User can switch between all three modes.
- [ ] Same object and scene settings are used for comparisons.
- [ ] Specular-highlight comparison is demonstrated.
- [ ] Light position and shininess can be changed, or prepared test configurations are available.
- [ ] Screenshots are captured.

#### Submission

- [ ] Report explains the algorithm and implementation.
- [ ] Report includes comparison results and limitations.
- [ ] Slides are complete.
- [ ] Final project builds from a clean checkout.
- [ ] Source code and presentation are backed up.

**Project rule:** Keep the existing renderer. Build the smallest complete implementation that clearly demonstrates why Phong shading differs from Flat and Gouraud shading.