#version 450


layout(set = 0, binding = 0) uniform View {
    mat4 viewMat;
    mat4 projMat;
    vec4 lightPos;
} view;

layout(push_constant) uniform Spatial {
  mat4 modelMat;
  mat4 modelInvMat;
} spatial;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

struct FragOut {
    vec4 color;
    vec3 normal;
    vec2 texcoord;
    vec3 viewDir;
    vec3 lightDir;
};

layout(location = 0) out FragOut fragout;

void main() {
    gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
    fragout.color = inColor;
    fragout.normal = inNormal.xyz;
    fragout.texcoord = inTexCoord;
    
    vec4 eyepos = view.viewMat * inPosition;
    fragout.normal = mat3(view.viewMat) * inNormal.xyz;
    vec3 lightPos = mat3(view.viewMat) * view.lightPos.xyz;
    fragout.lightDir = lightPos - eyepos.xyz;
    fragout.viewDir = -eyepos.xyz;
}

