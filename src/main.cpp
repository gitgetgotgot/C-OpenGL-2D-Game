#include <iostream>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.h"

//glfwSwapInterval(0); - remove FPS limiter

// VAO - vertex array object(to use different VBOs), VBO - vertex buffer object(for vertices), EBO - element buffer object (for indices)
// VAO first, then VBO, then EBO!
// Unbind in the same way

// local -> world (model matrix)
// world -> view (view matrix)
// view -> clip (projection matrix)
//clip -> screen
// Vclip = Mprojection * Mview * Mmodel * Vlocal

Game gameMain;

//window resize
static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
//keyboard
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//mouse cursor
static void cursor_position_callback(GLFWwindow* window, double mX, double mY);
//mouse buttons
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//mouse wheel scroll
static void mouse_wheel_callback(GLFWwindow* window, double xOffset, double yOffset);

std::unordered_map<int, char> shiftKeyMap = {
	{GLFW_KEY_1, '!'}, {GLFW_KEY_2, '@'}, {GLFW_KEY_3, '#'},
	{GLFW_KEY_4, '$'}, {GLFW_KEY_5, '%'}, {GLFW_KEY_6, '^'},
	{GLFW_KEY_7, '&'}, {GLFW_KEY_8, '*'}, {GLFW_KEY_9, '('},
	{GLFW_KEY_0, ')'}, {GLFW_KEY_MINUS, '_'}, {GLFW_KEY_EQUAL, '+'},
	{GLFW_KEY_LEFT_BRACKET, '{'}, {GLFW_KEY_RIGHT_BRACKET, '}'},
	{GLFW_KEY_BACKSLASH, '|'}, {GLFW_KEY_SEMICOLON, ':'},
	{GLFW_KEY_APOSTROPHE, '"'}, {GLFW_KEY_GRAVE_ACCENT, '~'},
	{GLFW_KEY_COMMA, '<'}, {GLFW_KEY_PERIOD, '>'}, {GLFW_KEY_SLASH, '?'}
};

int main() {
	srand(time(NULL));

	glfwInit();
	//version of OpenGL (version 4.6)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//using the core profile for only modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenGL Window", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);
	glfwSetWindowAspectRatio(window, 16, 9);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, mouse_wheel_callback);
	//glfwSwapInterval(0);

	gladLoadGL();

	//openGL version and GPU info
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	gameMain.init();

	std::chrono::steady_clock::time_point previous_time = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point current_time;

	while (!glfwWindowShouldClose(window)) {

		///INPUT///
		gameMain.input();
		glfwPollEvents();
		////

		////UPDATE////
		current_time = std::chrono::steady_clock::now();
		if (!gameMain.update(window, std::chrono::duration<float>(current_time - previous_time).count()))
			glfwSetWindowShouldClose(window, true);
		previous_time = current_time;
		////

		////RENDER////
		gameMain.render();
		////
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}

static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
	gameMain.ScreenWidth = width;
	gameMain.ScreenHeight = height;
	glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((mods & GLFW_MOD_SHIFT) && shiftKeyMap.find(key) != shiftKeyMap.end()) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
			gameMain.keyStates[int(shiftKeyMap[key])] = true;
		else if (action == GLFW_RELEASE)
			gameMain.keyStates[int(shiftKeyMap[key])] = false;
	}
	else
		gameMain.keyStates[key] = glfwGetKey(window, key);
}

static void cursor_position_callback(GLFWwindow* window, double mX, double mY) {
	gameMain.mouse.mouseX = (float)mX;
	gameMain.mouse.mouseY = gameMain.ScreenHeight - (float)mY;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	gameMain.mouse.left_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	gameMain.mouse.right_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
}

static void mouse_wheel_callback(GLFWwindow* window, double xOffset, double yOffset) {
	gameMain.mouse.wheelOffset = (float)yOffset;
}