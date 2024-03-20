#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 inPosition;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    float Re = 0, Im = 0, summaRe = 0, summaIm = 0, Arg = 0;
    float i = fragTexCoord.x * 512;
    for(int j = 0; j < 512; j++) {
        Arg = 2.0 * 3.14 * j * i / 512.0f;
        vec4 pixel = texture(texSampler ,vec2(i,fragTexCoord.y * 512));
        float intensive = sqrt(pixel[0]*pixel[0] * pixel[1]*pixel[1] * pixel[2]* pixel[2]);
        Re = cos(Arg)*intensive;
        Im = sin(Arg)*intensive;
        summaRe = summaRe + Re;
        summaIm = summaIm + Im;
    }
    float module = sqrt(summaRe*summaRe + summaIm*summaIm);
    outColor = vec4(module, module, module, 1.0f);
        
    
}