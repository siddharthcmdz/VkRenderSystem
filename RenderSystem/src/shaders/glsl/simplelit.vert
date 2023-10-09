#version 450

//vertex attributes
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

//descriptor sets
layout(set = 0, binding = 0) uniform View {
    mat4 viewMat;
    mat4 projMat;
    
} view;

layout(push_constant) uniform Spatial {
  mat4 modelMat;
  mat4 modelInvMat;
} spatial;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
