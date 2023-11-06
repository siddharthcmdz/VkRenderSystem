#version 450

struct FragDataOut {
	vec4 color;
};

layout(location = 0) in FragDataOut fragout;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = fragout.color;
}
