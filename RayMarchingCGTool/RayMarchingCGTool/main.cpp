// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>


#include "tinythread.hpp"
#include "block.hpp"
#include "windowinfo.hpp"
#include "appstate.hpp"
#include <cstring>
#include <chrono>
#include <vector>
	



static int RenderingThreadMain(void* data)
{
	// Initialize GLEW (a random context is required)
	glfwMakeContextCurrent(WindowInfoManager::getInstance().winInfoList[0]->window);
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(NULL);

	// Configure windows before rendering
	WindowInfoManager::getInstance().renderInitWindows();

	// Render loop
	int fpsCounter = 0;
	auto previousTime = std::chrono::system_clock::now();
	while (AppState::getInstance().isRunning) {
		WindowInfoManager::getInstance().renderWindows();

		fpsCounter++;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - previousTime).count() >= 1000) {
			printf("%d fps\n", fpsCounter);
			fpsCounter = 0;
			previousTime = std::chrono::system_clock::now();
		}
	}

	return 0;
}


void SetupUIThread() {

	DisplayWindowInfo::getInstance().SetupRC();
	DiagramWindowInfo::getInstance().SetupRC();
	
	if (thrd_create(&AppState::getInstance().uiThreadID, RenderingThreadMain, NULL) != thrd_success) {
		fprintf(stderr, "Failed to create Rendering thread\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
}

void DestroyUIThread() {
	int result;
	thrd_join(AppState::getInstance().uiThreadID, &result);

	DisplayWindowInfo::getInstance().DestroyRC();
	DiagramWindowInfo::getInstance().DestroyRC();
}


int main(int argc, char *argv[])
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	glfwSetErrorCallback(AppState::getInstance().error_callback);


	AppState::getInstance().isRunning = true;
	AppState::getInstance().resetTimer();

	// Create Display window/thread
	SetupUIThread();

	while (AppState::getInstance().isRunning) {
		// Poll events
		glfwPollEvents();
	}


	// Close Display thead and terminate GLFW
	DestroyUIThread(); 
	glfwTerminate();


	return 0;
}
