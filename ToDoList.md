# Rendering Engine
~~* Separate windowing system from rendersystem~~ **DONE**
~~* * send hwnd and hinstance to vkrs~~ **DONE**
~~* create API to resize a context~~ **DONE**
~~* Remove usage of glfw~~ **DONE**
~~* Make VkRenderSystem a dll and export only needed functions~~ **DONE**
* Support rendering luminance texture
* Support buffer naming to see in renderdoc and nsight
* Add support for macOS/iOS project **IN PROGRESS**

# Rendering techniques
* Support depth attachment in framebuffers by default
* Support IMGUI
* Implement VolumeDataModel that houses a 3D volume data
* Implement SliceProbes for axial, coronal and saggital
* Implement ObliqueProbe
* Implement 2D text rendering
* Implement 3D text rendering

# Scene Management
~~* Create architecture for running multiple examples~~ **DONE**
~~* Implement navigation mode~~ **DONE**
* Create a construct to render a 3D box grid
* Create constructs to render analytical geometry - cyclinder and cone
* Create a construct to render arrows representing X, Y and Z.
* Implement manipulation mode


# Rendering examples
~~* Render primitives like - triangle strip, triangles, line, line strip etc~~ **DONE**
** fix primitive example - use descriptors only for valid material, move command pool creation to iinstance 
~~* Render one quad sold, textured, points, lines~~ **DONE**


