#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec3 pos;


uniform int fogType = 2;


vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance ) // camera to point distance
{
    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}


void main()
{
    float dist = length(pos);
     if(fogType == 1) {
        color = vec4(applyFog(vec3(1.0), dist), 1.0);
    }
    if(fogType == 0) {
        color = vec4(1.0);
    }
}