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
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in vec2 inTexCoord;

struct FragDataOut {
	vec4 color;
	vec4 normal;
	vec2 texCoord;
};

//layout(location = 0) out vec4 fragColor;
//layout(location = 1) out vec4 fragNormal;
//layout(location = 2) out vec2 fragTexCoord;

layout(location = 0) out FragDataOut fragout;

void main() {
	gl_Position = view.projMat * view.viewMat * spatial.modelMat * inPosition;
	vec3 normal3 = (inNormal.xyz * 0.5f) + 0.5f);
	normal3 = normalize(normal3);
	vec4 normalColor = vec4(normal3.xyz, 1.0f);
	//fragout.color = normalColor;
	fragout.color = vec4(inTexCoord.xy, 0.5, 1.0f);
	fragout.normal = inNormal;
	fragout.texCoord = inTexCoord;

	gl_PointSize = 10.0f;
}
