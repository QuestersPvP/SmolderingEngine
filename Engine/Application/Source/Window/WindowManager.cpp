#include "WindowManager.h"



	bool WindowManager::InitWindowClass()
	{
		windowParams = GenerateApplicationWindowParameters();
		
		// Show the window (assuming windows OS)
		ShowWindow(windowParams.HWnd, SW_SHOWNORMAL);
		UpdateWindow(windowParams.HWnd);

		return true;
	}

	bool WindowManager::UpdateWindowClass(Renderer& _renderer)
	{
		MSG message;
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			// TODO: add event handeling (e.g. x button clicked, resized, etc.)
			switch (message.message)
			{
				// Application Events
			case USERMESSAGE_RESIZE:
				_renderer.ResizeWindow();
				break;
			case USERMESSAGE_QUIT:
				ApplicationShutdown();
				break;
				
				// Keydown Events
			case KEYDOWN_W:
				break;
			case KEYDOWN_A:
				break;
			case KEYDOWN_S:
				break;
			case KEYDOWN_D:
				break;
			case KEYDOWN_ESC:
				break;

				// Keyup events
			case KEYUP_W:
				break;
			case KEYUP_A:
				break;
			case KEYUP_S:
				break;
			case KEYUP_D:
				break;
			case KEYUP_ESC:
				break;
			default:
				std::cout << "Message not handled: " << message.message << std::endl;
				break;
			}

			TranslateMessage(&message);
			DispatchMessage(&message);
			
			return false;
		}
		return true;
	}

	void WindowManager::ApplicationShutdown()
	{
		SetApplicationRunStatus(false);
	}

