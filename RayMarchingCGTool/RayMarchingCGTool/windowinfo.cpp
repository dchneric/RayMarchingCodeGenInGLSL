#include "windowinfo.hpp"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "appstate.hpp"
#include "shader.hpp"

void WindowInfo::SetupRC() {
	glfwWindowHint(GLFW_SAMPLES, 9);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Initialize WINDOW
	window = glfwCreateWindow(Width, Height, "RaymarchingCGTool", NULL, NULL);

	if (!window) {
		fprintf(stderr, "Failed to open GLFW DisplayWindow. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

}

void WindowInfo::DestroyRC() {
	glfwDestroyWindow(window);
	window = NULL;
}

WindowInfo::WindowInfo(int w, int h) { Width = w; Height = h; window = NULL; }


void DisplayWindowInfo::key_callback(GLFWwindow* DisplayWindow, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(DisplayWindow, GL_TRUE);
}

void DisplayWindowInfo::resize_callback(GLFWwindow *DisplayWindow, int width, int height)
{
	getInstance().Height = height; getInstance().Width = width;
}

void DisplayWindowInfo::SetupRC()
{
	WindowInfo::SetupRC();

	// setup callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resize_callback); //glfwSetFramebufferSizeCallback
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}

void DisplayWindowInfo::UpdateShader(std::string fileName)
{
	// Update shaders (should throw if errors)
	GLint newProgramID = LoadShaders("DisplayWindow.vertexshader", fileName.c_str());

	// delete previous program
	glDeleteProgram(programID);
	programID = newProgramID;

	// Use our shader
	glUseProgram(programID);
}

void DisplayWindowInfo::RenderInit()
{
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("DisplayWindow.vertexshader", "Reference.fragmentshader");

	// Use our shader
	glUseProgram(programID);

	////////////// todo: add error checking

	timeID = glGetUniformLocation(programID, "time");
	resolutionID = glGetUniformLocation(programID, "resolution");

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	// VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// VAO
	glGenVertexArrays(1, &vertexarrayobject);
	glBindVertexArray(vertexarrayobject);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	// Dark blue background
	glViewport(0, 0, Width, Height);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
}


void DisplayWindowInfo::Render()
{
	if (needUpdateShader) {
		UpdateShader(AppState::OutputShaderName);
		needUpdateShader = false;
	}


	glViewport(0, 0, Width, Height);
	glClear(GL_COLOR_BUFFER_BIT);


	glUniform2f(resolutionID, Width, Height);
	glUniform1f(timeID, 0.001 * std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - AppState::getInstance().mtime).count());

	glBindVertexArray(vertexarrayobject);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw quad (2 triangles)
	glBindVertexArray(0);

	glfwSwapBuffers(window);
}

void DisplayWindowInfo::RenderTerm()
{
	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &vertexarrayobject);
}

void WindowInfoManager::renderInitWindows() const {

	for (auto it = WindowInfoManager::getInstance().winInfoList.begin(); it != WindowInfoManager::getInstance().winInfoList.end(); ++it) {
		glfwMakeContextCurrent((*it)->window);
		glfwSwapInterval(1);
		(*it)->RenderInit();
		glfwMakeContextCurrent(NULL);
	}

}

void WindowInfoManager::renderWindows() const{
	for (auto it = WindowInfoManager::getInstance().winInfoList.begin(); it != WindowInfoManager::getInstance().winInfoList.end(); ++it) {
		glfwMakeContextCurrent((*it)->window);


		(*it)->Render(); // todo: make needRedraw virtual, move swapBuffers out


		if (glfwWindowShouldClose((*it)->window))
			AppState::getInstance().isRunning = false;

		glfwMakeContextCurrent(NULL);
	}
}