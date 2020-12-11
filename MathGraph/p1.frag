#version 330 core

struct Light {
    vec3 pos;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
};

out vec4 color;

in vec3 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec3 pos;
in vec2 TexCoord;

uniform sampler2D ourTexture1;
//uniform sampler2D ourTexture2;

uniform vec3 objectColor;
uniform Light light;
uniform vec3 viewPos;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance ) // camera to point distance
{
    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}


vec3 applyFog1( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayDir,   // camera to point vector
               in vec3  sunDir )  // sun light direction
{

    float b = 0.4;
    float fogAmount = 1.0 - exp( -distance*b );
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( vec3(0.5,0.6,0.7), // bluish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,8.0) );
    return mix( rgb, fogColor, fogAmount );
}

void main()
{
    //color = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.2);

    //Phong 
    float amb = 0.1f;
    vec3 ambient = amb * light.color;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.pos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light.color;

    float distance = length(light.pos - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));

    ambient  *= attenuation; 
    diffuse  *= attenuation;
    specular *= attenuation;
    
    vec3 result = (ambient + diffuse + specular) * vec3(texture(ourTexture1, TexCoord));

    //Fog
    float dist = length(pos);
    vec3 posNorm = normalize(pos);


    color = vec4(result, 1.0f);
}