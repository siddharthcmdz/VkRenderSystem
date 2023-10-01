#version 450

//layout (location = 0) in vec4 fragColor;
//layout (location = 1) in vec4 fragNormal;
//layout (location = 2) in vec2 fragTexCoord;

struct FragDataOut {
	vec4 color;
	vec4 normal;
	vec2 texCoord;
};

layout(location = 0) in FragDataOut fragout;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = fragout.color;
	//outColor = vec4(fragout.texCoord.xy, 0.5f, 1.0f);
}
