#pragma once

#include <gl\glew.h>
#include <SOIL.h>


class Texture
{
public:
	GLuint texture;

	Texture(const GLchar* texturePath);
};

