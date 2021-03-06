﻿// MathGraph.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "SimpleModel.h"
#include <vector>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "random.h"
#include "VBOMesh.h"
#include "BMPReader.h"
#include <glm/gtc/noise.hpp>


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
unsigned int loadCubemap(std::vector<std::string> faces);
void renderShadowScene(const Shader& shader, GLint cubeVAO);
GLuint getParlTex(int twidth, int theight);


Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool keys[1024];

GLfloat deltaTime = 0.0f;	// Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;

GLfloat lastX = 400, lastY = 300;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLboolean firstMouse = true;
bool isSky = false;
int fog = 0;
bool isOpenwork = false;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glewExperimental = GL_FALSE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	

	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSetKeyCallback(window, key_callback);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Shader ourShader("./p1.ver", "./p1.frag");
	Shader skyShader("./skybox.ver", "./skybox.frag");
	Shader lightShader("./lamp.ver", "./lamp.frag");
	Shader smapShader("./shadow_map.ver", "./shadow_map.frag", "./shadow_map.geo");
	Shader normapShader("./norm_map.ver", "./norm_map.frag");
	Shader reflShader("./reflect.ver", "./reflect.frag");
	Shader refrShader("./refract.ver", "./refract.frag");

	Texture tex1("textures/container.jpg");
	Texture tex2("textures/awesomeface.png");
	GLuint ogre_diff = BMPReader::loadTex("textures/diffuse.bmp");
	GLuint ogre_norm = BMPReader::loadTex("textures/ogre_normalmap.bmp");
	GLuint parl_noise = getParlTex(400, 400);


	std::vector<std::string> faces
	{
		"./textures/right.jpg",
		"./textures/left.jpg",
		"./textures/top.jpg",
		"./textures/bottom.jpg",
		"./textures/front.jpg",
		"./textures/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	VBOMesh ogre = VBOMesh("./media/bs_ears.obj", false, true, true);
	VBOMesh pig = VBOMesh("./media/pig_triangulated.obj", false, true, true);

	glm::mat4 projection(1);
	projection = glm::perspective(glm::radians(45.0f), ((float)width/(float)height), 0.1f, 100.0f);


	SimpleModel smodel;
	smodel.initCube();
	SimpleModel skyM;
	skyM.initSkybox();
	SimpleModel plane;
	plane.initPlane();

	glm::vec3 lightPos(1.0f, 3.0f, 3.0f);
	glm::mat4 lModel(1);
	lModel = glm::translate(lModel, lightPos);
	lModel = glm::scale(lModel, glm::vec3(0.2f));



	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth cubemap texture
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//Init shader data for cubes
	ourShader.use();
	ourShader.setInt("ourTexture1", 0);
	ourShader.setInt("shadowMap", 1);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		do_movement();



		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 0. create depth cubemap transformation matrices
		// -----------------------------------------------
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj* glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// 1. render scene to depth cubemap
		// --------------------------------
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		smapShader.use();
		for (unsigned int i = 0; i < 6; ++i)
			smapShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		smapShader.setFloat("far_plane", far_plane);
		smapShader.setVec3("lightPos", lightPos);
		renderShadowScene(smapShader, smodel.VAO);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, 800, 600);




		// Render
		// Clear the colorbuffer
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		//Render Skybox
		if (isSky) {
			glDepthMask(GL_FALSE);
			skyShader.use();

			glm::mat4 view1 = glm::mat4(glm::mat3(camera.GetViewMatrix()));

			skyShader.setMat4("view", view1);
			skyShader.setMat4("projection", projection);

			glBindVertexArray(skyM.VAO);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glDepthMask(GL_TRUE);
		}


		//Draw cube
		glDisable(GL_CULL_FACE);
		ourShader.use();

		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		ourShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
		ourShader.setVec3("light.color", glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setVec3("light.pos", lightPos);
		ourShader.setFloat("light.constant", 1.0f);
		ourShader.setFloat("light.linear", 0.09f);
		ourShader.setFloat("light.quadratic", 0.032f);
		ourShader.setVec3("light.Intensity", vec3(0.9f, 0.9f, 0.9f));
		ourShader.setVec3("Material.Ks", 0.2f, 0.2f, 0.2f);
		ourShader.setVec3("Material.Ka", 0.1f, 0.1f, 0.1f);
		ourShader.setFloat("Material.Shininess", 1.0f);
		ourShader.setVec3("viewPos", camera.Position);
		ourShader.setFloat("far_plane", far_plane);
		ourShader.setBool("shadows", true);
		ourShader.setInt("fogType", fog);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1.texture);
		
		renderShadowScene(ourShader, smodel.VAO);


		//Draw light

		lightShader.use();

		lightShader.setMat4("model", lModel);
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		lightShader.setInt("fogType", fog);

		glBindVertexArray(smodel.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//Draw Reflect
		reflShader.use();
		glm::mat4 teaMod(1.0f);
		teaMod = glm::translate(teaMod, glm::vec3(2.0, 0.0, 0.0));
		teaMod = glm::scale(teaMod, glm::vec3(0.2));
		reflShader.setMat4("Model", teaMod);
		reflShader.setMat4("View", view);
		reflShader.setMat4("Projection", projection);
		reflShader.setMat3("NormalMatrix", glm::mat3(vec3(teaMod[0]), vec3(teaMod[1]), vec3(teaMod[2])));

		reflShader.setVec3("CamPos", camera.Position);

		reflShader.setBool("DrawSkyBox", !isSky);

		reflShader.setVec4("MaterialColor", vec4(0.5f, 0.5f, 0.5f, 1.0f));
		reflShader.setFloat("ReflectFactor", 0.85f);
		reflShader.setInt("fogType", fog);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		ogre.render();

		//Draw Rafract

		refrShader.use();
		glm::mat4 refrMod(1.0f);
		refrMod = glm::translate(refrMod, glm::vec3(-2.0, 0.0, 0.0));
		refrMod = glm::scale(refrMod, glm::vec3(0.2));
		refrShader.setMat4("Model", refrMod);
		refrShader.setMat4("View", view);
		refrShader.setMat4("Projection", projection);
		refrShader.setMat3("NormalMatrix", glm::mat3(vec3(refrMod[0]), vec3(refrMod[1]), vec3(refrMod[2])));

		refrShader.setVec3("CamPos", camera.Position);

		refrShader.setBool("DrawSkyBox", !isSky);

		reflShader.setVec4("MaterialColor", vec4(0.5f, 0.5f, 0.5f, 1.0f));
		refrShader.setFloat("Material.Eta", 0.94f);
		refrShader.setFloat("Material.ReflectionFactor", 0.1f);
		refrShader.setInt("fogType", fog);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		ogre.render();

		//Draw break



		//Draw normal maps and openworks

		glm::mat4 ogre_model = glm::mat4(1.0f);

		normapShader.use();

		normapShader.setVec3("Light.Position", lightPos);
		normapShader.setVec3("Light.Intensity", vec3(0.9f, 0.9f, 0.9f));
		normapShader.setVec3("Material.Ks", 0.2f, 0.2f, 0.2f);
		normapShader.setVec3("Material.Ka", 0.1f, 0.1f, 0.1f);
		normapShader.setFloat("Material.Shininess", 1.0f);
		normapShader.setInt("ColorTex", 0);
		normapShader.setInt("NormalMapTex", 1);
		normapShader.setInt("NoiseTex", 2);
		normapShader.setBool("Break", isOpenwork);
		normapShader.setFloat("LowThreshold", 0.45f);
		normapShader.setFloat("HighThreshold", 0.65f);
		normapShader.setMat4("View", view);
		normapShader.setMat4("Model", ogre_model);
		normapShader.setMat4("Projection", projection);
		normapShader.setMat3("NormalMatrix",
			glm::mat3(vec3(ogre_model[0]), vec3(ogre_model[1]), vec3(ogre_model[2])));
		normapShader.setVec3("viewPos", camera.Position);
		normapShader.setInt("fogType", fog);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ogre_norm);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ogre_diff);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, parl_noise);
		ogre.render();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 72 && action == GLFW_PRESS)
		isSky = !isSky;
	if (key == 66 && action == GLFW_PRESS)
		isOpenwork = !isOpenwork;

	if (key == 70 && action == GLFW_PRESS)
		fog = (fog + 1) % 2;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


void renderShadowScene(const Shader& shader, GLint cubeVAO)
{
	// room cube
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));
	shader.setMat4("model", model);
	glDisable(GL_CULL_FACE); 
	shader.setInt("reverse_normals", 1); 
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 24, 6);
	if (fog) {
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);
	shader.setInt("reverse_normals", 0); // and of course disable it
	glEnable(GL_CULL_FACE);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.0, -1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);


	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.5f, 3.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.75f));
	shader.setMat4("model", model);

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

GLuint getParlTex(int twidth, int theight) {

	GLubyte* data = new GLubyte[twidth * theight * 4];

	float xFactor = 1.0f / (twidth - 1);
	float yFactor = 1.0f / (theight - 1);

	for (int row = 0; row < theight; row++) {
		for (int col = 0; col < twidth; col++) {
			float x = xFactor * col;
			float y = yFactor * row;
			float sum = 0.0f;
			float freq = 7;
			float persist = 2;
			for (int oct = 0; oct < 4; oct++) {
				glm::vec2 p(x * freq, y * freq);

				float val = 0.0f;

				val = glm::perlin(p) * persist;


				sum += val;

				float result = (sum + 1.0f) / 2.0f;

				// Clamp strictly between 0 and 1
				result = result > 1.0f ? 1.0f : result;
				result = result < 0.0f ? 0.0f : result;

				// Store in texture
				data[((row * twidth + col) * 4) + oct] = (GLubyte)(result * 255.0f);
				freq *= 2.0f;
				persist *= 2;
			}
		}
	}

	GLuint texID;
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, twidth, theight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, twidth, theight, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	delete [] data;

	return texID;
}