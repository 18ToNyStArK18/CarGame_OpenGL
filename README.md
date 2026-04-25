# 3D Car Game
**Computer Graphics**

A dynamic 3D OpenGL game where the player controls a fast-moving car across an oval (two rectangles + two semi-circles) highway. Avoid procedural NPC traffic and buildings, utilize tactical boost mechanisms, and engage "bullet time" to weave through tight spots—all rendered with custom programmable shaders and a comprehensive dynamic light system.

---

## 🌟 Key Features & Graphics Engineering

### Advanced Lighting (Phong Reflection Model)
The custom shader pipeline (`shader.vs`, `shader.fs`) implements robust per-pixel lighting utilizing the Phong reflection model to achieve realistic visualization:
*   **Ambient Lighting**: A base illumination ensuring shadowed areas remain visible.
*   **Diffuse Lighting**: Calculates the primary directional brightness by analyzing the dot product of the surface normal and the light direction.
*   **Specular Highlights**: Simulates bright, shiny light reflections (specular reflection) off the car bodies  alongside the viewer's relative angle. Shininess relies on a robust constant parameter for distinct highlights.
*   **Multi-Light Architecture**: The shader actively evaluates up to 10 distinct dynamic light sources simultaneously, adapting well to moving street lights overhead.

### Model Loading & Texturing from Scratch
*   **Custom `.obj` & `.mtl` Parsing**: Bypasses external mesh-loading libraries by using a proprietary logic (`loadObjToFlatArray` & `loadUVToFlatArray`) which directly interprets flat raw strings exported natively from Blender.
*   **UV Mapping & Textures**: Powered by `stb_image.h`, translating native `.png`/.`jpg` into raw `GL_TEXTURE_2D` properties mapped perfectly onto building/car configurations.

### Gameplay Mechanics
*   **Collision Detection**: Utilizes Axis-Aligned Bounding Box (AABB) hitboxes wrapping every player, building, and NPC car to evaluate crash logic optimally every tick.
*   **Boost Mechanics**: Temporarily multiplies DeltaTime acceleration factors paired intrinsically with field-of-view dynamic scaling ("warp speed" aesthetic).
*   **Bullet Time Algorithm**: Re-maps global delta-time to fractional variables on an input interrupt, simulating extreme slow-motion capability allowing users to perform complex dodges.
*   **Dynamic Environments**: Incorporates aesthetic additions such as spinning windmills and mechanically swinging street lights (whose movements translate physically to the lighting coordinates passed to shaders).

---

## 🎥 Camera Pipeline
Five comprehensive digital viewports utilizing distinct LookAt view matrices:
1.  **Overhead / Bird's Eye**: Centered high pointing directly downwards.
2.  **First Person / Dash-cam**: Perpendicular vectors mounted immediately ahead of the player chassis aiming flatly through the world.
3.  **Side Cinematic**: Rotational perspective fixated beside bounding volumes. Customizable Field-of-View.
4.  **Swinging Light View**: A dynamic mounting bound precisely to oscillating street lights, providing sweeping scenes.
5.  **Default 3rd Person Follow**: Smooth interpolating chase cam resting predictably behind the driver.

---

## 🛠️ Dependencies & Setup

- OpenGL
- GLEW
- FreeGLUT
- SFML (Audio runtime logic exclusively)

**Ubuntu/Debian Installation Setup:**
```bash
sudo apt update
sudo apt install libglew-dev freeglut3-dev
sudo apt install libsfml-dev libsfml-audio-dev
```

---

## 🚀 Compiling and Running

Build the executable utilizing the simple Makefile structure:
```bash
make
```

Run the compiled artifact:
```bash
./arjuna_game
```
*(Ensure `shader.vs`, `shader.fs`, the `/models/` directory, and texture images reside in the relative path next to the game executable. Avoid relocating files outside defined relative paths.)*

---

## 🎮 Controls Map

| Input | Action Required |
|---|---|
| `L` / `R` | Steer vehicle Left or Right across the lane geometry |
| `F` / `S` | Increase or Decrease the Speed of the car |
| `1` - `5` | Hotkeys to toggle between the 5 unique Camera matrix perspectives |
| `R` | Restart stage / Clean simulation instance after triggering a fatal AABB Collision |
| `H` | Honk functional SFML audio queue mapping |
| `<` / `>` | Adjust Camera FOV bounds whilst inside 3rd-Mode isolated view |
| `Spacebar` | Engage NOS/Boost Mode (Overrides speed bounds & pushes Frustum depth mappings) |
| `T` | Trigger Bullet Time |
| `B` | Debug Mode: Render internal wireframe bounding box vectors (Hitboxes) for collision visualization |

---

## 📁 Repository Structure

```text
├── main.cpp        # Global render loop handles AABB bounding, lighting maths, matrices, etc.
├── file_utils.h    # Helper I/O file for binding shader string sources.
├── stb_image.h     # Fast standalone texture image interpreter library.
├── shader.vs       # Transformed Vertex pipelines (World/Perspective Matrix multiply)
├── shader.fs       # The fragment pipeline dealing explicitly with Phong reflections
├── Makefile        # C++ GCC compilation flags handling binary deployment
├── models/         # Blender Export .obj resources (Cars, Buildings, Generic Architecture)
├── textures/       # UV mapped sprite wrappers logic
├── *.mp3           # Raw SFML acoustic resources mapped to runtime events (collisons / horns)
└── README.md       # Root overview and execution notes
```
