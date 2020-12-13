#version 330 core

in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;
in vec3 pos;

uniform sampler2D ColorTex;
uniform sampler2D NormalMapTex;
uniform sampler2D NoiseTex;
uniform bool Break;
uniform float LowThreshold;
uniform float HighThreshold;
uniform int fogType = 2;

struct LightInfo {
  vec3 Position;  // Light position in eye coords.
  vec3 Intensity; // A,D,S intensity
};

uniform LightInfo Light;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo Material;

layout( location = 0 ) out vec4 FragColor;

vec3 phongModel( vec3 norm, vec3 diffR ) {
    vec3 r = reflect( -LightDir, norm );
    vec3 ambient = Light.Intensity * Material.Ka * diffR;
    float sDotN = max( dot(LightDir, norm), 0.0 );
    vec3 diffuse = Light.Intensity * diffR * sDotN;

    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = Light.Intensity * Material.Ks *
               pow( max( dot(r,ViewDir), 0.0 ), Material.Shininess );

    return ambient + diffuse + spec;
}

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance ) // camera to point distance
{
    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}

void main() {
    // Lookup the normal from the normal map
    vec4 normal = 2.0 * texture( NormalMapTex, TexCoord ) - 1.0;
    vec4 noise = texture( NoiseTex, TexCoord );
    float dist = length(pos);

    if(Break) {
        if( noise.a < LowThreshold ) discard;
        if( noise.a > HighThreshold ) discard;
    }

    vec4 texColor = texture( ColorTex, TexCoord );
    vec4 normcolor = vec4( phongModel(normal.xyz, texColor.rgb), 1.0 );
    if(fogType == 1) {
        FragColor = vec4(applyFog(normcolor.xyz, dist), 1.0);
    }
    if(fogType == 0) {
        FragColor = normcolor;
    }
}
