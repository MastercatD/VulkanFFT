#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 inPosition;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 intensive = texture(texSampler ,fragTexCoord + 0.5f);
    if (fragTexCoord.x >= 0.5f) {
        if (fragTexCoord.y >= 0.5f) {
            //intensive = texture(texSampler ,fragTexCoord + 0.5f);
            outColor = vec4(intensive[0], 0, 0, 1.0f);
        } else {
            //intensive = texture(texSampler ,fragTexCoord + 0.5f);
            outColor = vec4(0, intensive[1], 0, 1.0f);
        } 
    } else {
        if (fragTexCoord.y >= 0.5f) {
            //intensive = texture(texSampler ,fragTexCoord + 0.5f);
            outColor = vec4(0, 0, intensive[2], 1.0f);
        } else {
            //intensive = texture(texSampler ,fragTexCoord + 0.5f);
            outColor = vec4(intensive);  
        } 
    }
}