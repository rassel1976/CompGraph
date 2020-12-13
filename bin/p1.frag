#version 330 core

struct Light {
    vec3 pos;
    vec3 color;

    float constant;
    float linear;
    float quadratic;

    vec3 Intensity;
};

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};


out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 pos;
in vec2 TexCoord;

uniform sampler2D ourTexture1;
uniform samplerCube shadowMap;

uniform vec3 objectColor;
uniform Light light;
uniform vec3 viewPos;
uniform float far_plane;
uniform bool shadows;
uniform int fogType;
uniform MaterialInfo Material;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance ) // camera to point distance
{
    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}


float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - light.pos;  
    float closestDepth = texture(shadowMap, fragToLight).r;
    closestDepth *= far_plane;
    float currentDepth = length(fragToLight);
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;           
        
    return shadow;
}

vec3 phongModel( vec3 normal, vec3 diffR ) {
    vec3 LightDir = normalize(light.pos - FragPos);
    vec3 norm = normalize(normal);
    vec3 ViewDir = normalize(viewPos - FragPos);

    vec3 r = reflect( -LightDir, norm );
    vec3 ambient = light.Intensity * Material.Ka * diffR;
    float sDotN = max( dot(LightDir, norm), 0.0 );
    vec3 diffuse = light.Intensity * diffR * sDotN;

    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = light.Intensity * Material.Ks *
               pow( max( dot(r,ViewDir), 0.0 ), Material.Shininess );

    float distance = length(light.pos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));

    ambient  *= attenuation; 
    diffuse  *= attenuation;
    spec *= attenuation;

    float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;
    return ambient +  (1 - shadow) * (diffuse + spec);
}


void main()
{
    vec3 color = texture(ourTexture1, TexCoord).rgb;                   
    vec3 lighting = phongModel(Normal, color);   
    
    float dist = length(pos);
   

    if(fogType == 1) {
        FragColor = vec4(applyFog(lighting, dist), 1.0);
    }
    if(fogType == 0) {
        FragColor = vec4(lighting, 1.0);
    }
}