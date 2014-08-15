#pragma once

#ifndef WINDOWINFO_HPP
#define WINDOWINFO_HPP

#include <chrono>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include "mathutil.hpp"

class WindowInfo {

public:
	virtual void SetupRC();

	virtual void RenderInit() {};
	virtual void Render() = 0;
	virtual void RenderTerm() {};

	virtual void DestroyRC();

	int Height;
	int Width;
	GLFWwindow *window;

protected:
	WindowInfo(int w, int h); // always subclassing
};

class DisplayWindowInfo : public WindowInfo {

public:
	static DisplayWindowInfo &getInstance() {
		static DisplayWindowInfo instance(1600, 900);
		return instance;
	}

	virtual void SetupRC();
	virtual void RenderInit();
	virtual void Render();
	virtual void RenderTerm();

	bool needUpdateShader;

private:
	DisplayWindowInfo(int w, int h) : WindowInfo(w, h) { needUpdateShader = false; }

	static void key_callback(GLFWwindow* DisplayWindow, int key, int scancode, int action, int mods);
	static void resize_callback(GLFWwindow *DisplayWindow, int width, int height);

	GLuint programID;
	GLuint timeID;
	GLuint resolutionID;
	GLuint vertexbuffer;
	GLuint vertexarrayobject;

	// Update shader after compilation
	void UpdateShader(std::string fileName);
};


class Block;
class Connection;

class DiagramWindowUserInputManager {
public:
	enum UserInput { DRAG, SCALE, BLOCKDRAG, PORTDRAG, COMPILE, CANCEL };

	static void key_callback(GLFWwindow* DisplayWindow, int key, int scancode, int action, int mods);
	static void resize_callback(GLFWwindow *DisplayWindow, int width, int height);
	static void mousebutton_callback(GLFWwindow *DisplayWindow, int button, int action, int mods);
	static void mousemove_callback(GLFWwindow *DisplayWindow, double xPos, double yPos);
	static void mousescroll_callback(GLFWwindow* window, double xoffset, double yoffset);


	static void processInput(UserInput operation, GLFWwindow *DisplayWindow);
	static void updateInput(UserInput operation, GLFWwindow *DisplayWindow, double arg1 = 0, double arg2 = 0);
	static void cancelInput(UserInput operation, GLFWwindow *DisplayWindow);
	static void clearInput(GLFWwindow *DisplayWindow);

	static void startDragging(GLFWwindow *DisplayWindow);
	static void doDragging(GLFWwindow *DisplayWindow, double x, double y);
	static void stopDragging(GLFWwindow *DisplayWindow);

	static void startScaling(GLFWwindow *DisplayWindow);
	static void doScaling(GLFWwindow *DisplayWindow, double xoffset, double yoffset);
	static void stopScaling(GLFWwindow *DisplayWindow);

	static void startBlockDragging(GLFWwindow *DisplayWindow);
	static void doBlockDragging(GLFWwindow *DisplayWindow, double x, double y);
	static void stopBlockDragging(GLFWwindow *DisplayWindow);

	static bool updateConnectionForValidPort(Vec2 pos);
	static void doPortDragging(GLFWwindow *DisplayWindow, double x, double y);
	static void stopPortDragging(GLFWwindow *DisplayWindow);

	static void startCompiling(GLFWwindow *DisplayWindow);
	static void doCompiling(GLFWwindow *DisplayWindow);
	static void stopCompiling(GLFWwindow *DisplayWindow);

private:
	static UserInput currentUserInputState;
	static Vec2 pivotPos;
	static Block* pivotBlock;
	static Connection *pivotConnection;
	static bool pivotConnectionIsInputPort;
};

class DiagramWindowInfo : public WindowInfo {

public:
	static DiagramWindowInfo &getInstance() {
		static DiagramWindowInfo instance(800, 600);
		return instance;
	}

	virtual void SetupRC();
	virtual void RenderInit();
	virtual void Render();
	virtual void RenderTerm() {};

	GLuint programID;
	GLuint drawcolorID;
	GLuint mvpID;
	GLuint texturesamplerID;
	GLuint shadertypeID;

private:
	DiagramWindowInfo(int w, int h) : WindowInfo(w, h) { }

	void updateMVPIfNeeded();
	bool needRedraw();
	
	// Persistant DiagramWindowInfo Data
	float viewportScaleFactor;
	Vec2 viewportTopLeftCorner;
	bool needUpdateMVP;
	bool needRedrawDiagram;

	// Cache structures
	Rec DisplayArea;

	friend class DiagramWindowUserInputManager;
};







class WindowInfoManager {
public:
	static WindowInfoManager &getInstance() {
		static WindowInfoManager instance;
		return instance;
	}

	void renderInitWindows() const;

	void renderWindows() const;

	std::vector<WindowInfo *> winInfoList;

private:
	WindowInfoManager() : winInfoList() {
		// diagram window always open
		winInfoList.push_back(&DiagramWindowInfo::getInstance());
		// display window always open
		winInfoList.push_back(&DisplayWindowInfo::getInstance());
	}
};


#endif