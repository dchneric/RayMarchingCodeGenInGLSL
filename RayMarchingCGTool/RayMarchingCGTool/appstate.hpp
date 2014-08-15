#pragma once

#ifndef APPSTATE_HPP
#define APPSTATE_HPP

#include <chrono>
#include <string>
#include "tinythread.hpp"


// ¥Ê∑≈À˘”–persistent data
struct AppState {

	static const std::string OutputShaderName;

	static AppState &getInstance() {
		static AppState instance;
		return instance;
	}

	static void error_callback(int error, const char* description) {
		fputs(description, stderr);
	}

	void resetTimer() {
		mtime = std::chrono::system_clock::now();
	}

	std::chrono::time_point<std::chrono::system_clock> mtime;
	bool isRunning;
	thrd_t uiThreadID;

private:
	AppState() { isRunning = false; }
};



#endif