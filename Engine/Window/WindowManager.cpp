#include "WindowManager.h"

bool WindowManager::InitWindowClass()
{
	// TODO: MOVE WINDOW CREATING TO ANOTHER CLASS
	windowParams = GenerateApplicationWindowParameters();

	return true;
}

bool WindowManager::UpdateWindowClass()
{    
	// Show the window (assuming windows OS)
	ShowWindow(windowParams.HWnd, SW_SHOWNORMAL);
	UpdateWindow(windowParams.HWnd);

	MSG message;
	if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
	{
		// TODO: add event handeling (e.g. x button clicked, resized, etc.)

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return false;
}
