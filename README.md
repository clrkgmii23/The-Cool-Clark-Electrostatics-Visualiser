
## The Cool Clark Electrostatics Visualiser

A GPU accelerated real-time electrostatics visualiser, built using OpenGL in C++.

---
### Controls
Interaction in this project can be done using the mouse, keyboard, and editing in the ImGui Window, here are the keybinds:


- **SHIFT** + **Right Mouse Button Drag** &rarr; Move Camera.
- **Right Mouse Button Drag** &rarr; Rotate Camera.
- **Left Mouse Button** &rarr; Select And Drag.


And for the keyboard:
 - **SHIFT** + **P**, **L**, **C**, **R**, **S** &rarr; Each One Adds A Different Source Object.
 - **D** &rarr; Delete Selected Object;
 -  **X** OR **Y** OR **Z** &rarr; Restrict Next Drag Action Into An Axis.
 - **SHIFT** + **X** OR **Y** OR **Z** &rarr; Restrict Next Drag Action Into A Plane.
 - **TAB** Toggle Visualisation. 
 - **SPACE** Toggle Particle Simulation.

---
### What does it do?
This project can be used to display various charge sources, along with their contribution to a chosen field.
Currently this project contains these source objects: ***point charges, infinite lines, charged rings, infinite cylinders, and charged spheres**.* along with their *contributions* to the **Electric** and **Magnetic** field! 


This project supports three visualisation types: a **3D vector field** and **Streamlines**. both of these are togglable in the Dear ImGui window. along with a **particle simulation** using the field as forces controlling the movement of these particles.


All the computation is done in the **GPU** using **Compute Shaders**.

---
### How does it work?
I didn't have a solid plan when starting this, I was learning OpenGL and had an electrostatics class at the time so I thought it would be cool to combine the two. here is the basic idea that I came up with:

This Project is mainly composed of three different components: a **Compute Manager**  and A **Renderer**, each namely descriptive, along with a **Core**  used to pass data between the two. the Core starts up first, we pass it a user-defined list of source objects that can be updated by the user. it then configures both the Compute Manager and Renderer. starts up the compute manager first,  which by turn dynamically generates a compute shader based on the object list, visualisation type and field type. this compute shader will be run every frame, storing the needed visualisation data into an **SSBO buffer** to be passed for rendering. 


The renderer is pretty simple, it takes in the SSBO buffer of the visualisation data and the list of objects in the scene, and every frame it renders both the visualisation and source objects.


---
I tried to make it trivial adding new objects and visualisation fields. to add a new object you just need:
1. Create its Class in **SODefinitions.comp**
	- Define struct of data that will be used in the compute shader. and a StoreInBuffer function
	- Define its geometry in its **initialSetUp** function.
	- Define how it's drawn in its **Draw** function.
2. Write it's contribution to both the electric field and magnetic field (and whatever field you have added) in **SODefinitions.comp**.
3. if you want to be able to press a keybind to add this object, go to **Interactivity.h** and edit the *KeyToObj* map. (side note: The Common Shaders Class is defined in **Shader.h**)


(you can just use the ***point charge class*** as a simple example to all of this!)


If you want to add a new **field type**, it's even simpler! just go to **Interactivity.h**, scroll to the ***fields*** array, add whatever field you want, and then *define it for each object* in **SODefinitions.comp**.


If you mess up any of this the program will probably yell at you with errors and not crash.

---
### Tech Stack
pretty bare bones, this project is built using **C++** using these libraries:
- **OpenGL 4.3**
- **GLFW** and **GLAD** for window handling and OpenGL function loading.
- **GLM** for math
- **Dear ImGui** for simple gui

and that's about it:)
