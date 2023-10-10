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
    vec4 lightPos;
    vec4 viewPos;
} view;

layout(push_constant) uniform Spatial {
  mat4 modelMat;
  mat4 modelInvMat;
} spatial;

struct FragOut {
    vec4 color;
    vec3 normal;
    vec2 texcoord;
    vec3 viewDir;
    vec3 lightDir
};


layout(location = 0) out FragOut fragout;

void main() {
    gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
    fragout.color = inColor;
    fragout.normal = inNormal;
    fragout.texcoord = inTexCoord;

    vec4 eyepos = view.viewMat * inPosition;
    fragout.normal = mat3(view.viewMat) * inNormal.xyz;
    vec3 lightPos = mat3(view.viewMat) * view.lightPos.xyz;
    fragout.lightDir = view.lightPos.xyz - eyepos.xyz;
    fragout.viewDir = view.viewPos.xyz - eyepos.xyz;
}
