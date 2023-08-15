#version 450

//vertex attributes
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inTexCoord;

//descriptor sets
layout(set = 0, binding = 0) uniform View {
    mat4 viewMat;
    mat4 projMat;
    
} view;

/*
layout(set = 2, binding = 0) uniform Spatial {
  mat4 modelMat;  
} spatial;
*/

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    //gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
    gl_Position = view.projMat * view.viewMat * inPosition;
    fragColor = inColor;
    fragTexCoord = inTexCoord.xy;
}
