
#version 450

layout(set = 0, binding = 0) uniform View {
    mat4 viewMat;
    mat4 projMat;
    vec4 lightPos;
} view;

layout(push_constant) uniform Spatial {
    mat4 modelMat;
    mat4 modelInvMat;
    mat4 textureMat;
} spatial;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragTexCoord;

void main() {
    gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
    vec4 volumeUVW = spatial.textureMat * inPosition;
    fragTexCoord = volumeUVW.xyz;
}

