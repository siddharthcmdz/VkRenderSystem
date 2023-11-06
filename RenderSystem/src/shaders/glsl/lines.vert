#version 450

//descriptor sets
layout(set = 0, binding = 0) uniform View {
    mat4 viewMat;
    mat4 projMat;
    
} view;

layout(push_constant) uniform Spatial {
  mat4 modelMat;
  mat4 modelInvMat;
} spatial;

layout(location = 0) in vec4 inPosition;
layout(location = 2) in vec4 inColor;

struct FragDataOut {
	vec4 color;
};

layout(location = 0) out FragDataOut fragout;

void main() {
	gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
	fragout.color = inColor;
}
