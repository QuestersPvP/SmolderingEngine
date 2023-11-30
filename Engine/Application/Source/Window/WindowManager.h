#pragma once

#include "../Common/Common.h"
#include "../Rendering/Application/WindowCreation.h"
#include "Rendering/Renderer.h"

using namespace SmolderingEngine;

class WindowManager
{
	/* Variables */

public:
	WindowParameters windowParams; // TODO: MOVE WINDOW CREATING TO ANOTHER CLASS
	//InputManager windowInput;

private:
	bool applicationShouldRun = true;

	/* Functions */

public:
	bool InitWindowClass();
	bool UpdateWindowClass(Renderer& _renderer);

	const WindowParameters GetWindow() { return windowParams; };

	const void SetApplicationRunStatus(bool _newRunStatus) { applicationShouldRun = _newRunStatus; };
	const bool GetApplicationRunStatus() { return applicationShouldRun; };

private:
	void ApplicationShutdown();

};



