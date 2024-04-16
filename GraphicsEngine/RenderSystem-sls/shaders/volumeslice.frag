
#version 450
//#define DEBUG_OUTLIERS

layout(location = 0) in vec3 fragTexCoord;

layout(set = 1, binding = 0) uniform usampler3D texSampler;
layout(set = 1, binding = 1) uniform VolumeSlice 
{
    vec2 window; //window[0] is window width and window[1] is window height
    vec2 rescale; //rescale[0] is m and rescale[1] is b
} volumeSlice;

layout(location = 0) out vec4 outColor;


void main()
{
    const float windowWidth = volumeSlice.window[0];
    const float windowCenter = volumeSlice.window[1];
    const float m = volumeSlice.rescale[0];
    const float b = volumeSlice.rescale[1];
    
    if(fragTexCoord.x < 0.0f || fragTexCoord.x > 1.0f || fragTexCoord.y < 0.0f || fragTexCoord.y > 1.0f || fragTexCoord.z < 0.0f || fragTexCoord.z > 1.0f)
    {
#ifdef DEBUG_OUTLIERS
        outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
#else
        discard;
#endif
    }
    else
    {
        uint smple = texture(texSampler, fragTexCoord).r;
        
        const float halfWidth = 0.5f * windowWidth;
        float gray = 0.0f;
        float hu = float(smple) * m + b;
        float nsmple = hu;
        if(nsmple <= windowCenter - halfWidth)
        {
            gray = 0.0f;
        }
        else if(nsmple > windowCenter + halfWidth)
        {
            gray = 1.0f;
        }
        else
        {
            float start = windowCenter - halfWidth;
            float end = windowCenter + halfWidth;
            gray = mix(start, end, nsmple);
            gray = (nsmple - start)/(end-start);
        }
        
        outColor = vec4(gray, gray, gray, 1.0f);
    }
}

