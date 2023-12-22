#pragma once

#include "../Common/Common.h"
#include "../Rendering/Application/WindowCreation.h"
#include "Rendering/Renderer.h"

using namespace SmolderingEngine;

class WindowManager
{
	/* Variables */

public:
	WindowParameters windowParams;

private:
	bool applicationShouldRun = true;
	bool wButtonDown = false;
	bool aButtonDown = false;
	bool sButtonDown = false;
	bool dButtonDown = false;

	uint32_t targetWidth = 1280;
	uint32_t targetHeight = 720;

	/* Functions */

public:
	bool InitWindowClass();
	bool UpdateWindowClass(Renderer& _renderer);

	const WindowParameters GetWindow() { return windowParams; };

	const void SetApplicationRunStatus(bool _newRunStatus) { applicationShouldRun = _newRunStatus; };
	const bool GetApplicationRunStatus() { return applicationShouldRun; };

private:
	void ApplicationShutdown();
	void ProcessInput(Renderer& _renderer);

};



