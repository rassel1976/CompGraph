#pragma once

#include <gl\glew.h>
#include "stb_image.h"
#include <vector>
#include <string>
#include <iostream>


class Texture
{
public:
	GLuint texture;

	Texture(const GLchar* texturePath);
};

