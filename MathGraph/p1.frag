#version 330 core

struct Light {
    vec3 pos;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
};

out vec4 FragColor;

in vec3 ourColor;
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

void main()
{
    vec3 color = texture(ourTexture1, TexCoord).rgb;
    float amb = 0.3f;
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
      
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(FragPos) : 0.0;                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;   
    
    float dist = length(pos);
    
    vec3 newColor = mix(lighting, ourColor, 0.4);

    if(fogType == 1) {
        FragColor = vec4(applyFog(lighting, dist), 1.0);
    }
    if(fogType == 2) {
        FragColor = vec4(newColor, 1.0);
    }
    if(fogType == 0) {
        FragColor = vec4(lighting, 1.0);
    }
}