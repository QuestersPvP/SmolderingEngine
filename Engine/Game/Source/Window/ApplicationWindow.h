#pragma once

#include "../../Renderer/Source/Common/Common.h"

using namespace SmolderingEngine;

class ApplicationWindow
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

