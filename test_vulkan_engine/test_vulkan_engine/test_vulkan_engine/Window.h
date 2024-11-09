#pragma once

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <GLFW\glfw3.h>

#include "Camera.h"

class Window
{
public:
	GLFWwindow* mainWindow;

	Window();
	Window(int windowWidth, int windowHeight, std::string winName);

	bool* getsKeys() { return keys; }
	float getXChange();
	float getYChange();

	~Window();

private:

	int width, height;
	int bufferWidth, bufferHeight;

	bool keys[1024];

	float lastX;
	float lastY;
	float xChange;
	float yChange;
	bool mouseFirstMoved;

	void createCallbacks();
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
};

