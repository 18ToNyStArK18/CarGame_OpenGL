# The Test of Arjuna
**CSE7.302: Computer Graphics — Assignment 1**

A 2D OpenGL game inspired by the Draupadī-svayamvara: shoot a ball through the gaps of a rotating wheel to hit the bird above it.

---

## Dependencies

- OpenGL
- GLEW
- FreeGLUT
- SFML (Audio)

On Ubuntu:
```bash
sudo apt install libglew-dev freeglut3-dev
sudo apt-get install libsfml-dev libsfml-audio-dev
```

---

## Compiling

```bash
make
```

---

## Running

```bash
./arjuna_game
```

The shader files `shader.vs` and `shader.fs` must be in the same directory as the executable.

---

## Controls

| Key / Input | Action |
|---|---|
| `F` / `S` | Increase / decrease canon speed  |
| `s` / `S` | Increase / decrease wheel RPM |
| 'L' / 'R' | Move left / right |
| '1','2','3','4','5' | Change the camera angle |
| `R` | Restart after game over (when the car is crashed) |
| `H` | To sound Horn |
| `<` / '>' | To change the camera angle when the camera is in the 3rd mode |
| ' ' | For Boost |






---


---

## Extras Implemented

- **Horn** — Press 'H' to sound Horn
- **Street Lights** — Added multiple street light
- **Boost Mode** — if you press Space it will increase the speed of the car by 5 times and changes the fov creating a boost effect
- **Sound Effects** - Sound effects when collisoion happend
- **Good looking car and streetlights** - Imported the models from blender and imported the obj file and parsed it in the code itself

---

## File Structure

```
├── main.cpp        # Main game logic and rendering
├── file_utils.h    # Shader file reading utility
├── shader.vs       # Vertex shader
├── shader.fs       # Fragment shader
├── music.mp3       # Sound effect for wheel/spoke hits
├── Makefile
├── model           #Has all the obj files for the models
├── textures        #has all the texturepack in this dir
└── README.md

```
