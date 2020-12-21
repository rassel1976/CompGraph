#version 330

in vec3 ReflectDir;
in vec3 pos;

uniform samplerCube CubeMapTex;

uniform bool DrawSkyBox;
uniform float ReflectFactor;
uniform vec4 MaterialColor;
uniform int fogType = 2;

layout( location = 0 ) out vec4 FragColor;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance ) // camera to point distance
{
    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}


void main() {
    // Access the cube map texture
    //invert x and z of ReflectDir
    vec4 cubeMapColor = texture(CubeMapTex, vec3(ReflectDir.x, ReflectDir.y, ReflectDir.z));
    vec4 resColor = vec4(1.0);
    float dist = length(pos);
    if( DrawSkyBox )
        resColor = MaterialColor;
    else
        resColor = mix( MaterialColor, cubeMapColor, ReflectFactor);

    if(fogType == 1) {
        FragColor = vec4(applyFog(resColor.xyz, dist), 1.0);
    }
    if(fogType == 0) {
        FragColor = resColor;
    }
}
