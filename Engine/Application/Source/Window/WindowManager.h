#pragma once

#include "Utilities/Includes/ApplicationIncludes.h"
#include "../Rendering/Application/WindowCreation.h"
#include "Rendering/Renderer.h"

using namespace SE_Renderer;

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

	bool firstMouseUpdate = true;
	uint32_t mouseX = 0;
	uint32_t mouseY = 0;
	glm::vec2 mousePos = glm::vec2();

	/* Functions */

public:
	bool InitWindowClass();
	bool UpdateWindowClass(Renderer& _renderer);

	const WindowParameters GetWindow() { return windowParams; };

	const void SetApplicationRunStatus(bool _newRunStatus) { applicationShouldRun = _newRunStatus; };
	const bool GetApplicationRunStatus() { return applicationShouldRun; };

	void UpdateInput(Renderer& _renderer, float _time);
	void UpdateRotation(Renderer& _renderer);

private:
	void ApplicationShutdown();

};



