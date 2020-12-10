// MathGraph.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <iostream>
#include <SOIL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "SimpleModel.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
void drawCube(Shader shader, glm::mat3 model, glm::mat3 view, glm::mat3 projection, SimpleModel cube);

Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool keys[1024];

GLfloat deltaTime = 0.0f;	// Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;

GLfloat lastX = 400, lastY = 300;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLboolean firstMouse = true;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
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

	glm::vec3 cubePositions[] = {
  glm::vec3(0.0f,  0.0f,  0.0f),
  glm::vec3(2.0f,  5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3(2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3(1.3f, -2.0f, -2.5f),
  glm::vec3(1.5f,  2.0f, -2.5f),
  glm::vec3(1.5f,  0.2f, -1.5f),
  glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	Shader ourShader("./p1.ver", "./p1.frag");
	Shader lightShader("./p1.ver", "./lamp.frag");
	Texture tex1("container.jpg");
	Texture tex2("awesomeface.png");

	glm::mat4 projection(1);
	projection = glm::perspective(glm::radians(45.0f), ((float)width/(float)height), 0.1f, 100.0f);


	SimpleModel smodel;
	smodel.initCube();

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	glm::mat4 lModel(1);
	lModel = glm::translate(lModel, lightPos);
	lModel = glm::scale(lModel, glm::vec3(0.2f));

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();
		do_movement();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		ourShader.Use();

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");

		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		GLint objectColorLoc = glGetUniformLocation(ourShader.Program, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(ourShader.Program, "lightColor");
		glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

		GLint lightPosLoc = glGetUniformLocation(ourShader.Program, "lightPos");
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);

		GLint viewPosLoc = glGetUniformLocation(ourShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex1.texture);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2.texture);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture2"), 1);*/
		glBindVertexArray(smodel.VAO);
		for (GLuint i = 0; i < 1; i++)
		{
			glm::mat4 model(1);
			model = glm::translate(model, cubePositions[i]);
			GLfloat angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		int k = 7;

		lightShader.Use();

		GLint lmodelLoc = glGetUniformLocation(lightShader.Program, "model");
		glUniformMatrix4fv(lmodelLoc, 1, GL_FALSE, glm::value_ptr(lModel));

		GLint lviewLoc = glGetUniformLocation(lightShader.Program, "view");
		glUniformMatrix4fv(lviewLoc, 1, GL_FALSE, glm::value_ptr(view));

		GLint lprojLoc = glGetUniformLocation(lightShader.Program, "projection");
		glUniformMatrix4fv(lprojLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(smodel.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
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

void drawCube(Shader shader, glm::mat3 model, glm::mat3 view, glm::mat3 projection, SimpleModel cube) {
	GLint modelLoc = glGetUniformLocation(shader.Program, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	GLint viewLoc = glGetUniformLocation(shader.Program, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	GLint projLoc = glGetUniformLocation(shader.Program, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLint objectColorLoc = glGetUniformLocation(shader.Program, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(shader.Program, "lightColor");
	glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

	shader.Use();
	glBindVertexArray(cube.VAO);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}