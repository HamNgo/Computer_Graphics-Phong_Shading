# Phong Shading Project: 14-Day Roadmap

| | |
|---|---|
| **Team size** | 3 people |
| **Environment** | C++ with the existing GLFW project (`main.cpp`) |
| **Objective** | Implement Phong shading and compare it fairly with Flat and Gouraud shading |
| **Time limit** | 2 weeks (14 days) |

---

## 1. Project scope and decisions

### Main deliverable

A working C++ application that renders the same 3D object under three selectable shading modes:

1. **Flat shading**: one lighting value per triangle, computed from its face normal.
2. **Gouraud shading**: lighting computed at each vertex, with the resulting colors interpolated across the triangle.
3. **Phong shading**: vertex normals interpolated across the triangle, with lighting computed per pixel.

Phong shading is the main subject of the project. Flat and Gouraud shading are comparison baselines.

### Keep the existing setup

- Continue with the current C++ / GLFW renderer. Do not switch graphics libraries.
- Reuse the existing camera, vector math, window, input handling, and projection maths wherever possible.
- Do not add texture mapping, shadows, PBR, complex model loading, or a sophisticated GUI unless the core project is already complete.

### Feasibility check (result of reviewing `main.cpp`)

| Needed capability | Current state | Consequence |
|---|---|---|
| Filled polygons | Yes, via `GL_POLYGON` | Usable for Flat and Gouraud only |
| Depth testing | Yes, handled by the OpenGL depth buffer | Not accessible from our code |
| Per-pixel access / barycentric weights | **No**: the fixed-function pipeline hides rasterization | **Phong cannot be done with the current drawing code** |
| Normals, materials, lights | **No**: `drawFaces` uses a view-angle brightness approximation, not real lighting | Must be added |
| Triangle mesh | **No**: faces are quads plus one hexagon, with no normals | Must triangulate; sphere must be added |

**Decision:** build a minimal CPU triangle rasterizer with its own framebuffer and Z-buffer, and display the finished image with `glDrawPixels` (available in legacy OpenGL without extra libraries). All three shading modes then run through this one rasterizer, which also makes the comparison fairer. Using GLSL shaders would be simpler, but it requires a function loader, which breaks the "no additional libraries" rule.

Do not build a general-purpose graphics engine. The rasterizer needs only: triangle coverage, barycentric weights, a Z-buffer, and a pixel write.

### Reuse from `main.cpp`

- `Vec3` and the helpers (`subtract`, `dotProduct`, `crossProduct`, `vectorLength`). Add `add`, `scale`, and `normalize`.
- `computeCamera()` and the eye basis (`eyePosition`, `rightDirection`, `upDirection`, `forwardDirection`).
- The projection maths in `sendVertexToScreen`. Refactor it into a function that **returns** the screen position, the depth for the Z-buffer, and the eye-space depth, instead of calling `glVertex3f` directly. Perspective and parallel modes must keep working. Output is in the range [-1, 1] and must be mapped to the square pixel viewport.
- The lamp transform (`transformLampPoint`) and all camera and lamp controls.
- Draw the axes and the wireframe mode as before, using OpenGL lines on top of the framebuffer image (or a simple software line routine).

### Code organization

`main.cpp` is currently one file. Three people editing it at once will cause merge conflicts, so on Day 2 split it into a few headers. Suggested split:

- `vec3.h`: vector math
- `mesh.h`: mesh data, triangulation, normals
- `lighting.h`: `Material`, `Light`, `calculatePhongLighting`
- `raster.h`: framebuffer, Z-buffer, triangle fill

---

## 2. Team responsibilities

Assign one person to each area. Everyone helps with integration and testing near the end.

| Member | Main responsibility | Concrete deliverables |
|---|---|---|
| **Member 1: Geometry and rasterization** | Mesh data, face normals, vertex normals, triangle coverage, depth testing | Sphere (and triangulated lamp) as a triangle mesh; correct face and vertex normals; software rasterizer with Z-buffer that exposes per-pixel barycentric weights |
| **Member 2: Shading algorithms** | Shared Phong illumination equation; Flat, Gouraud, and Phong modes | One reusable lighting function and three working shading modes |
| **Member 3: Integration and evaluation** | Projection refactor, framebuffer display, controls, test scenes, screenshots, report and slides | Shading-mode selection, consistent comparison scenes, recorded results, documentation |

**Member 3 is not only a report writer.** The projection refactor, framebuffer display, comparison modes, and test setup are technical project work.

### Shared interface agreement

Agree on common data structures and coordinate conventions before coding. Adapt these examples to the existing types rather than rewriting working code.

```cpp
struct Color { float r, g, b; };

struct Vertex {
    Vec3 position;   // world space
    Vec3 normal;     // world space, normalized
};

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

struct Light {
    Vec3 position;   // world space
    Vec3 color;
};

enum ShadingMode { FLAT, GOURAUD, PHONG };
```

One shared lighting function:

```cpp
Color calculatePhongLighting(
    Vec3 position,
    Vec3 normal,
    Vec3 cameraPosition,
    Light light,
    Material material
);
```

**Coordinate convention: world space.** Positions, normals, the light position, and `eyePosition` must all be in world space (the lamp is already transformed to world space in `transformedVertices`). Document any deviation.

### Pitfalls specific to this project

- **Normals and non-uniform scale.** The lamp can be scaled differently along each axis (`lampScaleX/Y/Z`). Normals must **not** be transformed like positions in that case: divide by the scale on each axis (inverse transpose), then rotate, then normalize.
- **Light is fixed in world space** and does not move with the lamp unless we decide otherwise. State this in the report.
- **Winding and normal direction.** The current code takes `fabs` of the dot product, so inconsistent face winding is hidden. Real lighting needs outward-facing normals. Check winding, or flip each normal so it points away from the object's centre.
- **Lamp has sharp edges.** Averaging adjacent face normals would wrongly round off its box and shade corners. Use the **sphere** as the main test object. If the lamp is also shown, give it per-face (duplicated) vertices or an angle threshold for smoothing.

---

## 3. Fourteen-day schedule

### Days 1-2: Inspect, agree, and prepare

**All members**
- Build and run the current project before changing anything.
- Confirm the feasibility results above and agree on the software-rasterizer decision.
- Agree on coordinate conventions, data structures, and function signatures.
- Choose the test object: a sphere (latitude/longitude mesh with adjustable resolution).
- Split `main.cpp` into headers and agree on how code changes will be merged (shared branch, small commits).

**Member 1**
- Plan the triangle mesh representation and the rasterizer interface.

**Member 2**
- Review the Phong illumination equation and prepare pseudocode.
- Confirm the lighting function's inputs and coordinate space.

**Member 3**
- Define the three comparison modes and prepare a simple test checklist.
- Prepare the README/report outline and screenshot naming convention.

**Milestone:** Everyone can build the same starting project, and the team knows exactly what is missing.

### Days 3-4: Geometry and baseline rendering

**Member 1**
- Generate the sphere as triangles; triangulate the lamp (quads split into two triangles, the hexagon as a fan, about 50 triangles in total).
- Calculate each triangle's face normal.
- Generate smooth vertex normals by averaging adjacent face normals (area-weighted if straightforward), then normalize.
- Start the software rasterizer: bounding box, barycentric coverage test, Z-buffer, and pixel write.

**Member 2**
- Add the vector operations needed by the lighting equation.
- Implement and test the shared ambient, diffuse, and specular lighting function using known vectors.
- Clamp final color values to the valid display range.

**Member 3**
- Refactor the projection function so it returns screen position and depth values.
- Create the framebuffer and display it with `glDrawPixels` (handle window resizing and the square viewport).
- Prepare a fixed camera, light, and material configuration.
- Decide the controls (see Section 4).

**Milestone:** The sphere renders as filled, depth-tested triangles in the new rasterizer (any solid color), and the team can inspect valid face and vertex normals.

### Days 5-6: Flat and Gouraud shading

**Member 1**
- Finish the rasterizer, including Z-buffer integration.
- Expose the triangle's vertices and per-pixel barycentric weights to the shading code.

**Member 2**
- Flat shading: evaluate lighting once per triangle using its face normal and centroid.
- Gouraud shading: evaluate lighting at the three vertices and interpolate the resulting colors across the triangle.

**Member 3**
- Connect the mode selector and the light and shininess controls.
- Verify that Flat and Gouraud modes use the same mesh, camera, light, and material.

**Milestone:** Flat and Gouraud modes work. Keep this as a stable, tagged checkpoint before adding Phong shading.

### Days 7-9: Phong shading

**Member 1**
- Provide the three vertex normals and barycentric weights for each covered pixel.
- Provide the interpolated world-space position for each pixel (needed for the view and light directions).

**Member 2**
- Phong shading: interpolate the vertex normals across each triangle, normalize the interpolated normal, then evaluate lighting per pixel.
- Use the same lighting function as Flat and Gouraud.
- Check the specular term carefully; normalize the light, view, and normal vectors.

**Member 3**
- Integrate the Phong mode into the selector.
- Prepare a test with a small, bright specular highlight (high shininess, low-resolution sphere) so the difference between Gouraud and Phong is visible.

**Milestone:** All three shading modes render the same object.

### Days 10-11: Debugging and fair comparison

**All members**
- Fix visible artifacts, incorrect normal directions, inconsistent coordinate spaces, and depth errors.
- Compare modes with identical scene settings.
- Test at least two shininess values and two light positions.
- Record limitations, such as perspective interpolation or performance.

**Member 1**
- Verify mesh normals and Z-buffer behavior (including both projection modes).

**Member 2**
- Verify lighting behavior and compare the highlight in Gouraud versus Phong.

**Member 3**
- Capture comparison screenshots and record observations.

**Milestone:** The three modes are stable enough to demonstrate.

### Days 12-13: Report and presentation

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

### Day 14: Final verification and demonstration

- Build and run the final version.
- Demonstrate Flat, Gouraud, and Phong shading on the same object.
- Change the light position and shininess.
- Explain the visual and computational differences.
- Keep a backup copy of the source code and final presentation.

**Final milestone:** A reproducible demonstration and complete project submission.

---

## 4. Implementation reference

### Controls

Existing keys must keep working: `Tab` (camera/lamp mode), `P` (projection), `1` (wireframe), `2` (faces), `R`, `WASDEQ`, arrows, mouse wheel, `Esc`. Suggested additions (adjust to taste):

| Key | Action |
|---|---|
| `2` / `3` / `4` | Flat / Gouraud / Phong (`1` stays wireframe) |
| `I` `K` / `J` `L` / `U` `O` | Move the light along y / x / z |
| `[` / `]` | Decrease / increase shininess |
| `M` | Switch between low- and high-resolution sphere |

Show the current shading mode, shininess, and light position in the window title, as the existing code does for other values.

### Shared Phong illumination

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

The main difference between the modes is **where the lighting equation is evaluated**.

### Flat shading

1. Calculate one face normal for the triangle.
2. Choose a representative position for the triangle (for example, its centroid).
3. Evaluate lighting once using that face normal.
4. Fill the triangle with the resulting color.

### Gouraud shading

1. Obtain a normal at each vertex.
2. Evaluate lighting separately at the three vertices.
3. Interpolate the three resulting vertex colors across the triangle.
4. Write the interpolated color to each covered pixel.

### Phong shading

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

4. Evaluate the Phong illumination equation at that pixel using the interpolated normal and the pixel's world-space position.

**Important:** Phong shading does not mean interpolating the final colors. It means interpolating normals first, then calculating lighting separately at each pixel.

### Depth and perspective

- The existing code already outputs a depth value suitable for the Z-buffer: `-1/depth` under perspective, and a scaled `depth` under parallel projection. Both can be interpolated linearly in screen space.
- Screen-space barycentric interpolation of normals, colors, and positions is a practical starting point. Under perspective projection, **perspective-correct** interpolation (interpolating attribute/depth and 1/depth, then dividing) is more accurate. Treat it as an optional improvement if time is limited, and note the limitation in the report. Parallel projection needs no correction.

---

## 5. Comparison and evaluation plan

Use the **same object, camera, light, material, and display settings** for each shading mode. Change only the shading mode.

| Test | Setup | What to observe |
|---|---|---|
| Basic appearance | Same sphere and neutral light | Faceted Flat shading versus smoother Gouraud and Phong shading |
| Specular highlight | Small, bright highlight | Gouraud may miss a highlight between vertices; Phong evaluates lighting across the surface |
| Shininess | Test a low and a high exponent | Highlight size and intensity |
| Light movement | Move the light around the sphere | Whether illumination changes smoothly and consistently |
| Mesh resolution | Compare low- and higher-polygon spheres | How mesh density affects visible faceting and smoothness |

### Suggested results table

Fill this in after running the tests; do not invent results.

| Observation | Flat | Gouraud | Phong |
|---|---|---|---|
| Smooth appearance | | | |
| Small specular highlights | | | |
| Visible polygon boundaries | | | |
| Relative computation cost | | | |

### Expected general findings

- Flat shading visibly emphasizes individual polygon faces.
- Gouraud shading smooths color transitions but can miss small specular highlights.
- Phong shading usually produces smoother and more accurate specular highlights because lighting is evaluated per pixel.
- Phong shading generally requires more lighting calculations than Gouraud shading.

If performance is measured (for example, frame time), use the same resolution, mesh, and hardware, and report the measurement method.

---

## 6. Minimum viable submission

If time becomes tight, prioritize in this order:

1. A working filled-triangle renderer with depth testing.
2. Correct face and vertex normals.
3. A working shared Phong illumination function.
4. Flat, Gouraud, and Phong shading modes.
5. A fair side-by-side comparison.
6. Report and presentation polish.

Do **not** sacrifice a working Phong implementation to add optional features.

### Features to skip unless everything above is complete

- Texture mapping
- Shadows and shadow mapping
- Physically based rendering
- Complex imported models
- Advanced GUI controls
- Multiple light types
- Additional graphics libraries
- Perspective-correct interpolation (optional improvement)

---

## 7. Final team checklist

### Core implementation

- [ ] Existing C++/GLFW project builds and runs.
- [ ] `main.cpp` is split into headers and everyone can merge cleanly.
- [ ] Software rasterizer draws filled triangles with a Z-buffer.
- [ ] Face normals are correct and outward-facing.
- [ ] Smooth vertex normals are correct.
- [ ] Normals are transformed correctly under non-uniform lamp scaling.
- [ ] Flat shading works.
- [ ] Gouraud shading works.
- [ ] Phong shading interpolates and normalizes normals per pixel.
- [ ] All modes use consistent lighting and material settings.
- [ ] Camera, lamp, and projection controls still work.

### Demonstration

- [ ] User can switch between all three modes.
- [ ] Same object and scene settings are used for comparisons.
- [ ] Specular-highlight comparison is demonstrated.
- [ ] Light position and shininess can be changed, or prepared test configurations are available.
- [ ] Screenshots are captured.

### Submission

- [ ] Report explains the algorithm and implementation.
- [ ] Report includes comparison results and limitations.
- [ ] Slides are complete.
- [ ] Final project builds from a clean checkout.
- [ ] Source code and presentation are backed up.

**Project rule:** Keep the existing renderer. Build the smallest complete implementation that clearly demonstrates why Phong shading differs from Flat and Gouraud shading.
