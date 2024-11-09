#include "Window.h"


Window::Window()
{
	this->width = 800;
	this->height = 600;

	for (int i = 0; i < 1024; i++)
	{
		this->keys[i] = 0;
	}

	this->xChange = 0.0f;
	this->yChange = 0.0f;
}

Window::Window(int windowWidth, int windowHeight, std::string winName = "Window")
{
	this-> width = windowWidth;
	this->height = windowHeight;

	for (int i = 0; i < 1024; i++)
	{
		this->keys[i] = 0;
	}

	this->xChange = 0.0f;
	this->yChange = 0.0f;

	// Initialise GLFW
	glfwInit();

	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	this->mainWindow = glfwCreateWindow(width, height, winName.c_str(), nullptr, nullptr);

	glfwGetFramebufferSize(mainWindow, &this->bufferWidth, &this->bufferHeight);

	// handel key and mouse input
	createCallbacks();
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetWindowUserPointer(mainWindow, this);

}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}
void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::handleMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}


void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, handleKeys);
	glfwSetCursorPosCallback(mainWindow, handleMouse);
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}