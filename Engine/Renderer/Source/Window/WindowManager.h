#pragma once

#include "../Common/Common.h"
#include "../Rendering/Application/WindowCreation.h"

using namespace SmolderingEngine;

class WindowManager
{
	/* Variables */

public:
	WindowParameters windowParams; // TODO: MOVE WINDOW CREATING TO ANOTHER CLASS

	/* Functions */

public:
	bool InitWindowClass();
	bool UpdateWindowClass();

	const WindowParameters GetWindow() { return windowParams; };

};

