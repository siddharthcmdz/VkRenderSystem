
#version 450

layout(location = 0) in vec3 fragTexCoord;

layout(set = 1, binding = 0) uniform sampler3D texSampler;

layout(location = 0) out vec4 outColor;

void main() 
{
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    vec3 dim = textureSize(texSampler, 0);
    dim = dim - vec3(1, 1, 1);

    if(fragTexCoord.x < 0.0f || fragTexCoord.y < 0.0f || fragTexCoord.z < 0.0f)
    {
        discard;
    }

    vec3 normalizedUVW = fragTexCoord/dim;

    vec4 smple = texture(texSampler, normalizedUVW);
    outColor = vec4(smple.r, smple.r, smple.r, 1.0f);
    
}
