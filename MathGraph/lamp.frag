#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec3 pos;


uniform int fogType = 2;

void main()
{
     color = vec4(1.0);
}