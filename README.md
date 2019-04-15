# Games Graphics Programming Assignment 
Please read this document before trying to open this
## Controls
|Action|Control|
|---|---|
|Rotate the cube about the X axis|Press 'R' and then 'X'|
|Rotate the cube about the Y axis|Press 'R' and then 'Y'|
|Rotate the cube about the Z axis|Press 'R' and then 'Z'|
|Render the cube in wireframe mode|Press 'W'|
|Render the cube in solid mode|Press 'S'|
|Cull all Back Facing sides (you wont notice anything happen when you do this but it's there)|Press 'B'|
|Cull all Front Facing sides|Press 'F'|
|Cull nothing(You won't notice this happen either)|Press 'N'|
|Move the camera freely|Arrow Keys|
|Move the cube freely|Left click and drag inside the window|
|View the cube front on|Press '1'|
|View the cube top down|Press '2'|
|View the cube side on|Press '3'|
|View the cube side from left point|Press '0'|
|Reset the cube|Press 'I'|
|Rotate the front face 90°|Press '4'|
|Rotate the left face 90°|Press '5'|
|Rotate the right face 90°|Press '6'|
|Rotate the back face 90°|Press '7'|
|Rotate the top face 90°|Press '8'|
|Rotate the bottom face 90°|Press '9'|
## To Open
In order to open this you need
 - Visual Studio 2015 or later
 - C++ with Windows SDK later than 10.015... installed
 - DirectX 12
 - A DirectX 12 compatible GPU (This was written between a GTX 1080 and an RX570)
## The Task
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
