# Games Graphics Programming Assignment 

This assignment was designed to test the understanding students have of the Rendering pipeline and how 3D graphics are processed. 
The task was to make a functional rubiks with the following functional elements
  - The location of the cubes relative to one another can be altered by twisting the
    outer third of the cube by 90°, 180° or 270°.
- The user can reset the cube to the initial solved state by pressing the ‘I’ key.
- The user can start and stop the cube rotating across one or more axis by
pressing the ‘R’ key followed by selecting the appropriate axis with the X, Y and
Z keys.
- The user should be able to change the rendering mode (solid, wireframe) by
pressing the ‘S’ and ‘W’ keys respectively.
- The user can dynamically switch between the different cull modes (none, front,
back) by pressing ‘N’, ‘F’ and ‘B’ respectively.
- Simple camera control should be implemented allowing the user to navigate
around the world using the keyboard and mouse.
- The user should be able to select predefined default camera views of the cube
(front, top, side) by pressing the keys 1, 2 and 3 respectively.
- The player can score points by matching 3 correctly coloured cubes in a
horizontal or vertical row.

My final solution implemented these

|Feature|Implemented|
|---|---|
|The location of the cubes relative to one another can be altered by twisting the outer third of the cube by 90°, 180° or 270°.|:x:|
|The user can reset the cube to the initial solved state by pressing the ‘I’ key.|:heavy_check_mark:|
|The user can start and stop the cube rotating across one or more axis by pressing the ‘R’ key followed by selecting the appropriate axis with the X, Y and Z keys.|:heavy_check_mark:|
|The user should be able to change the rendering mode (solid, wireframe) by pressing the ‘S’ and ‘W’ keys respectively.|:heavy_check_mark:|
|The user can dynamically switch between the different cull modes (none, front, back) by pressing ‘N’, ‘F’ and ‘B’ respectively.|:heavy_check_mark:|
|Simple camera control should be implemented allowing the user to navigate around the world using the keyboard and mouse.|:heavy_check_mark:|
|The user should be able to select predefined default camera views of the cube (front, top, side) by pressing the keys 1, 2 and 3 respectively.|:heavy_check_mark:|
|The player can score points by matching 3 correctly coloured cubes in a horizontal or vertical row.|:x:|

## Building Blocks
The required reading for this module is [Introduction to 3D Game Programming with DirectX 12 - Luna, Frank D. - 2016 ](http://www.d3dcoder.net/ "D3DCoder.net"), as such, source code that initialises Direct3D and gets handles to the GPU etc... are code from his D3D Utility classes, the module only covers Buffers, Indexed Primitives, Texture Mapping, Constant Buffers, Shaders and Vector & Matrix algebra.
