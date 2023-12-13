#include "WindowManager.h"



	bool WindowManager::InitWindowClass()
	{
		windowParams = GenerateApplicationWindowParameters();

		// Show the window (assuming windows OS)
		ShowWindow(windowParams.HWnd, SW_SHOW);
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
				wButtonDown = true;
				break;
			case KEYDOWN_A:
				aButtonDown = true;
				break;
			case KEYDOWN_S:
				sButtonDown = true;
				break;
			case KEYDOWN_D:
				dButtonDown = true;
				break;

				// Keyup events
			case KEYUP_W:
				wButtonDown = false;
				break;
			case KEYUP_A:
				aButtonDown = false;
				break;
			case KEYUP_S:
				sButtonDown = false;
				break;
			case KEYUP_D:
				dButtonDown = false;
				break;
			default:
				std::cout << "Message not handled: " << message.message << std::endl;
				break;
			}

			TranslateMessage(&message);
			DispatchMessage(&message);
			
			return false;
		}

		// Handle input events
		ProcessInput(_renderer);

		return true;
	}

	void WindowManager::ApplicationShutdown()
	{
		SetApplicationRunStatus(false);
	}

	void WindowManager::ProcessInput(Renderer& _renderer)
	{
		if (wButtonDown)
		{
			_renderer.AddToTranslationZValue(0.01f);
		}
		else if (aButtonDown)
		{
			_renderer.AddToTranslationXValue(-0.01f);
		}
		else if (sButtonDown)
		{
			_renderer.AddToTranslationZValue(-0.01f);
		}
		else if (dButtonDown)
		{
			_renderer.AddToTranslationXValue(0.01f);
		}
	}

