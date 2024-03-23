#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 inPosition;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

/*
void DFT() {
    // Высота и ширина
    int width = 512, height = 512;
    float Re1d = 0, Im1d = 0, summaRe1d = 0, summaIm1d = 0, Arg1d = 0;
    float i = fragTexCoord.x * 512;
    for(int k = 0; k < 512; k++) {
        Arg1d = 2.0 * 3.14 * j * k / 512.0f;


        vec4 pixel1d = texture(texSampler ,vec2(j, k));
        float intensive1d = sqrt(pixel[0]*pixel[0] * pixel[1]*pixel[1] * pixel[2]* pixel[2]);
        Re1d = cos(Arg)*intensive;
        Im1d = sin(Arg)*intensive;
        summaRe1d = summaRe1d + Re1d;
        summaIm1d = summaIm1d + Im1d;
    }
    float module = sqrt(summaRe*summaRe + summaIm*summaIm);
    outColor = vec4(summaRe, summaRe, summaRe, 1.0f);
        
    
}*/

void main() {
    // Высота и ширина
    int width = 512, height = 512;
    float Re = 0, Im = 0, summaRe = 0, summaIm = 0, Arg = 0;
    float i = fragTexCoord.x * 512;
    for(int j = 0; j < 512; j++) {
        Arg = 2.0 * 3.14 * j * i / 512.0f;
        vec4 pixel = texture(texSampler ,vec2(j,fragTexCoord.x * 512));
        float intensive = sqrt(pixel[0]*pixel[0] * pixel[1]*pixel[1] * pixel[2]* pixel[2]);
        /*
        float Re1d = 0, Im1d = 0, summaRe1d = 0, summaIm1d = 0, Arg1d = 0;
        float i = fragTexCoord.x * 512;
        for(int k = 0; k < 512; k++) {
            Arg1d = 2.0 * 3.14 * j * k / 512.0f;
            vec4 pixel1d = texture(texSampler ,vec2(j, k));
            float intensive1d = sqrt(pixel[0]*pixel[0] * pixel[1]*pixel[1] * pixel[2]* pixel[2]);
            Re1d = cos(Arg)*intensive;
            Im1d = sin(Arg)*intensive;
            summaRe1d = summaRe1d + Re1d;
            summaIm1d = summaIm1d + Im1d;
        }*/
        /* Ограничение?
        if (summaRe1d > 1) {
            summaRe1d = 1;
        }
        if (summaIm1d > 1) {
            summaIm1d = 1;
        }*/
        /*
        Re = cos(Arg)*summaRe1d;
        Im = sin(Arg)*summaIm1d;
        */
        Re = cos(Arg)*intensive;
        Im = sin(Arg)*intensive;
        summaRe = summaRe + Re;
        summaIm = summaIm + Im;
    }
    float module = sqrt(summaRe*summaRe + summaIm*summaIm);
    outColor = vec4(module, module, module, 1.0f);
        
    
}
