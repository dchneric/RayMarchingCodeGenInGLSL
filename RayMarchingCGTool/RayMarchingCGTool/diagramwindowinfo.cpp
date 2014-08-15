#include "windowinfo.hpp"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "appstate.hpp"
#include "block.hpp"
#include "renderingtarget.hpp"


void DiagramWindowInfo::SetupRC()
{
	WindowInfo::SetupRC();

	// setup callbacks
	glfwSetKeyCallback(window, DiagramWindowUserInputManager::key_callback);
	glfwSetWindowSizeCallback(window, DiagramWindowUserInputManager::resize_callback); //glfwSetFramebufferSizeCallback
	glfwSetMouseButtonCallback(window, DiagramWindowUserInputManager::mousebutton_callback);
	glfwSetCursorPosCallback(window, DiagramWindowUserInputManager::mousemove_callback);
	glfwSetScrollCallback(window, DiagramWindowUserInputManager::mousescroll_callback);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}

void DiagramWindowInfo::RenderInit()
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0.01);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("DiagramWindow.vertexshader", "DiagramWindow.fragmentshader");

	// Use our shader
	glUseProgram(programID);

	drawcolorID = glGetUniformLocation(programID, "drawColor");
	mvpID = glGetUniformLocation(programID, "MVP");
	texturesamplerID = glGetUniformLocation(programID, "myTextureSampler"); glUniform1i(texturesamplerID, 0); // TEXUTRE0, never changed
	shadertypeID = glGetUniformLocation(programID, "shaderType"); glUniform1i(shadertypeID, 0); // 0 by default


	// Persistant data
	viewportScaleFactor = 1.0f;
	viewportTopLeftCorner = Vec2(-Width / 2.0f / viewportScaleFactor, +Height / 2.0f / viewportScaleFactor);

	needUpdateMVP = true;
	needRedrawDiagram = true;


	// set background color
	glClearColor(rtConstants::backgroundColor[0], rtConstants::backgroundColor[1], rtConstants::backgroundColor[2], rtConstants::backgroundColor[3]);
}

void DiagramWindowInfo::updateMVPIfNeeded()
{
	if (needUpdateMVP) {

		float halfTrueWidth = Width / 2.0f / viewportScaleFactor;
		float halfTrueHeight = Height / 2.0f / viewportScaleFactor;

		// Projection matrix
		glm::mat4 Projection = glm::ortho(-halfTrueWidth, halfTrueWidth, -halfTrueHeight, halfTrueHeight, 0.0f, 10.0f);
		// Camera matrix
		glm::mat4 View = glm::lookAt(
			glm::vec3(viewportTopLeftCorner.x + halfTrueWidth, viewportTopLeftCorner.y - halfTrueHeight, 1), // Camer in World Space
			glm::vec3(viewportTopLeftCorner.x + halfTrueWidth, viewportTopLeftCorner.y - halfTrueHeight, 0), // looks at the origin
			glm::vec3(0, 1, 0)  // Head is up
			);
		// Model matrix : an identity matrix (model will be at the origin, changing is forbidden)
		const glm::mat4 Model = glm::mat4(1.0f);
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &(Projection * View * Model)[0][0]);

		// update viewport (most of updateMVP are triggered by viewport size change)
		glViewport(0, 0, Width, Height);

		// update display area
		DisplayArea = { viewportTopLeftCorner.x, viewportTopLeftCorner.y - Height / viewportScaleFactor, Width / viewportScaleFactor, Height / viewportScaleFactor };

		needUpdateMVP = false;
	}
}

bool DiagramWindowInfo::needRedraw()
{
	return needUpdateMVP || needRedrawDiagram;
}

void DiagramWindowInfo::Render()
{
	if (needRedraw()){
		needRedrawDiagram = false;

		glClear(GL_COLOR_BUFFER_BIT);
		
		updateMVPIfNeeded();

		for (auto it = BlockGraph::getInstance().blockOrderList.begin(); it != BlockGraph::getInstance().blockOrderList.end(); ++it) {
			if (!dynamic_cast<Block*>(*it) || overlaps(DisplayArea, dynamic_cast<Block*>(*it)->renderRec)) {
				(*it)->DrawObject();
			}
		}

		glfwSwapBuffers(window);
	}
}