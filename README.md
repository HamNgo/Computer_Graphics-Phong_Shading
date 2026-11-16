# Phong Shading Project: 14-Day Roadmap

| | |
|---|---|
| **Team size** | 3 people |
| **Environment** | C++ with the existing GLFW project |
| **Objective** | Implement Phong shading and compare it fairly with Flat and Gouraud shading |
| **Time limit** | 2 weeks (14 days) |

---

## 1. Project scope and decisions

### Main deliverable

A working C++ application that renders the same 3D object under three selectable shading modes:

1. **Flat shading**: one lighting value per triangle, computed from its face normal.
2. **Gouraud shading**: lighting computed at each vertex, with the resulting vertex colors interpolated across the triangle.
3. **Phong shading**: vertex normals interpolated across the triangle, with lighting computed per pixel.

Phong shading is the main subject of the project. Flat and Gouraud shading are comparison baselines.

### Keep the existing setup

- Continue with the current C++ / GLFW renderer. Do not switch to another graphics library during this project.
- Reuse the existing camera, projection, vector math, window, and depth-buffer code wherever possible.
- Do not add texture mapping, shadows, PBR, complex model loading, or a sophisticated GUI unless the core project is already complete.

### Important feasibility check

Before implementation, verify whether the current renderer can already:

- draw filled triangles (not only points or lines);
- determine which pixels lie inside a triangle;
- perform depth testing with a Z-buffer;
- write a color to an individual pixel.

If these features already exist, reuse them. If filled-triangle rasterization is missing, make a minimal software triangle rasterizer the first priority. Do not build a general-purpose graphics engine.

---

## 2. Team responsibilities

Assign one person to each area. Everyone helps with integration and testing near the end.

| Member | Main responsibility | Concrete deliverables |
|---|---|---|
| **Member 1: Geometry and rasterization** | Mesh data, face normals, vertex normals, triangle coverage, depth testing | A sphere/mesh rendered as filled triangles; correct face and vertex normals; per-pixel access to triangle data |
| **Member 2: Shading algorithms** | Shared Phong illumination equation; Flat, Gouraud, and Phong modes | One reusable lighting function and three working shading modes |
| **Member 3: Integration and evaluation** | Connecting modes to the application; controls; test scenes; screenshots; report and slides | Shading-mode selection, consistent comparison scenes, recorded results, documentation |

**Member 3 is not only a report writer.** The comparison modes and test setup are technical project work.

### Shared interface agreement

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

## 3. Fourteen-day schedule

### Days 1-2: Inspect, agree, and prepare

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

### Days 3-4: Geometry and baseline rendering

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

### Days 5-6: Flat and Gouraud shading

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

### Days 7-9: Phong shading

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

### Days 10-11: Debugging and fair comparison

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

The main difference between the three modes is **where the lighting equation is evaluated**.

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

4. Evaluate the Phong illumination equation at that pixel using the interpolated normal and the pixel's position.

**Important:** Phong shading does not mean interpolating the final colors. It means interpolating normals first, then calculating lighting separately at each pixel.

### Practical implementation note

For a first software-renderer implementation, screen-space barycentric interpolation is a practical starting point. Under perspective projection, perspective-correct interpolation is more accurate, but it can be treated as an optional improvement if time is limited.

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

If performance is measured, use the same resolution, mesh, and hardware conditions, and report the measurement method.

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

---

## 7. Final team checklist

### Core implementation

- [ ] Existing C++/GLFW project builds and runs.
- [ ] A filled 3D mesh renders with depth testing.
- [ ] Face normals are correct.
- [ ] Smooth vertex normals are correct.
- [ ] Flat shading works.
- [ ] Gouraud shading works.
- [ ] Phong shading interpolates and normalizes normals per pixel.
- [ ] All modes use consistent lighting and material settings.

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
