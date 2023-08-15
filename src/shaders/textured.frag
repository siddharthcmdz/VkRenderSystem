
#version 450

//inputs 
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

//descriptor sets
layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
