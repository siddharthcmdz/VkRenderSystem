
#version 450

//inputs 
struct FragOut {
    vec4 color;
    vec3 normal;
    vec2 texcoord;
    vec3 viewDir;
    vec3 lightDir
};

layout(location = 0) in FragOut fragout;

//descriptor sets
layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(texSampler, fragout.texcoord) * vec4(fragout.color.xyz, 1.0f);
    
    vec3 N = normalize(fragout.normal);
    vec3 L = normalize(fragout.lightDir);
    vec3 V = normalize(fragout.viewDir);
    vec3 R = reflect(L, N);
    vec3 diffuse = max(dot(N, L), 0.15) * fragout.color;
    vec3 specular = pow(max(dot(R, V), 0.0f), 16.0f) * vec3(0.75);

    outColor = vec4(diffuse * color.rgb + specular, 1.0f);
}
