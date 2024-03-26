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
    int width = 2024, height = 2024;
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
    float width = 4096, height = 4096;
    
   /*
    float Re = 0, Im = 0, summaRe = 0, summaIm = 0, Arg = 0;
    float x = fragTexCoord.x * width, y = fragTexCoord.y * height;
    for(int j = 0; j < height; j++) {
        Arg = 2.0 * 3.14 * j * y /  height - 1;
        vec4 pixel = texture(texSampler ,vec2(x, j));
        float intensive = pixel[0];
        
        float Re1d = 0, Im1d = 0, summaRe1d = 0, summaIm1d = 0, Arg1d = 0;
        for(int k = 0; k < width; k++) {
            Arg1d = 2.0 * 3.14 * x * k / width - 1;
            vec4 pixel1d = texture(texSampler ,vec2(k, j));
            //float intensive1d = sqrt(pixel[0]*pixel[0] * pixel[1]*pixel[1] * pixel[2]* pixel[2]);
            float intensive1d = pixel1d[0];
            Re1d = cos(Arg1d)*intensive1d;
            Im1d = sin(Arg1d)*intensive1d;
            summaRe1d = summaRe1d + Re1d;
            summaIm1d = summaIm1d + Im1d;
        }
        
         Ограничение?
        if (summaRe1d > 1) {
            summaRe1d = 1;
        }
        if (summaIm1d > 1) {
            summaIm1d = 1;
        }
        
        Re = cos(Arg)  *summaRe1d + sin(Arg) * summaIm1d;
        Im = sin(Arg) * summaRe1d + cos(Arg) * summaIm1d;
        
        //Re = cos(Arg)*intensive;
        //Im = sin(Arg)*intensive;
        summaRe = summaRe + Re;
        summaIm = summaIm + Im;
    }
    float module = summaIm;
    //float module = sqrt(summaRe*summaRe + summaIm*summaIm);
    outColor = vec4(module,module, module, 1.0f);
    
    */
    float Re = 0, Im = 0, summaRe = 0, summaIm = 0, Arg = 0;
    float x = fragTexCoord.x * width, y = fragTexCoord.y * height;
    // Преобразование по строкам
    
    float a = 2.0 * 3.14 / (width - 1);
    for (int n = 0; n < width; n++) {
        vec4 pixel = texture(texSampler, vec2(n, y));
        float xn = pixel[0];
        Arg = a * n * x;
        Re = cos(Arg) * xn;
        Im = sin(Arg) * xn;
        summaRe = summaRe + Re;
        summaIm = summaIm + Im;
    }
    // Преобразование по столбцам
    /*
    float a = 2.0 * 3.14 / (width - 1);
    for (int n = 0; n < height; n++) {
        vec4 pixel = texture(texSampler, vec2(x, n));
        float xn = pixel[0];
        Arg = a * n * y;
        Re = cos(Arg) * xn;
        Im = sin(Arg) * xn;
        summaRe = summaRe + Re;
        summaIm = summaIm + Im;
    }
    */
    float module = summaRe;
    outColor = vec4(module,module, module, 1.0f);
    /*if (x != y && x != height - y && x != width / 2 && y != height / 2 && x != 0 && y != 1) {
    outColor = vec4(module,module, module, 1.0f);
    outColor = vec4(0, 0, 0, 1.0f);
    }
    else {
    outColor = vec4(255,0, 0, 1.0f);
    }*/
    
}
