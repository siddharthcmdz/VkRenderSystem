#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

void main() {
	//gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.xyz, 1.0);
	//fragColor = inColor.rgb;
	gl_Position = vec4(inPosition.xyz, 1.0);
	fragColor = vec4(1, 0, 0, 1);
}
