#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec3 pos;
in vec3 ourColor;


uniform int fogType = 2;

void main()
{
    if(fogType == 2) {
        color = vec4(1.0) * vec4(ourColor, 1.0);
    }else {
        color = vec4(1.0);
    }
}