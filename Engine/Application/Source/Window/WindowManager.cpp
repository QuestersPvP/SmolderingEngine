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
			switch (message.message)
			{
				// Application Events
			case USERMESSAGE_RESIZE:
				if (_renderer.GetApplicationReadyToRender() && (message.wParam != SIZE_MINIMIZED))
				{
					targetWidth = LOWORD(message.lParam);
					targetHeight = HIWORD(message.lParam);
					_renderer.ResizeWindow(targetWidth, targetHeight);
				}
				break;
			case USERMESSAGE_QUIT:
				ApplicationShutdown();
				break;
			case USERMESSAGE_MOUSEMOVE:
				mouseX = LOWORD(message.lParam);
				mouseY = HIWORD(message.lParam);
				UpdateRotation(_renderer);
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

		return true;
	}

	void WindowManager::ApplicationShutdown()
	{
		SetApplicationRunStatus(false);
	}

	void WindowManager::UpdateInput(Renderer& _renderer, float _time)
	{
		// get the current camera pos
		glm::vec3 updatedPos = _renderer.camera.position;

		// Handle position
		if (wButtonDown)
		{
			//_renderer.AddToTranslationZValue(0.01f);
			updatedPos.z += _renderer.camera.movementSpeed * _time;
		}
		else if (aButtonDown)
		{
			//_renderer.AddToTranslationXValue(-0.01f);
			updatedPos.x += _renderer.camera.movementSpeed * _time;
		}
		else if (sButtonDown)
		{
			//_renderer.AddToTranslationZValue(-0.01f);
			updatedPos.z -= _renderer.camera.movementSpeed * _time;
		}
		else if (dButtonDown)
		{
			//_renderer.AddToTranslationXValue(0.01f);
			updatedPos.x -= _renderer.camera.movementSpeed * _time;
		}

		// update the camera location
		_renderer.camera.SetPosition(updatedPos);
	}

	void WindowManager::UpdateRotation(Renderer& _renderer)
	{
		// check to see if the mouse has not moved before (this stops the first movement from flipping camera)
		if (firstMouseUpdate)
		{
			mousePos.x = mouseX;
			mousePos.y = mouseY;
			firstMouseUpdate = false;
		}
		
		glm::vec3 updatedRot = _renderer.camera.rotation;

		// handle rotation
		int32_t dx = (int32_t)mousePos.x - mouseX;
		int32_t dy = (int32_t)mousePos.y - mouseY;
		mousePos = glm::vec2((float)mouseX, (float)mouseY);

		updatedRot += glm::vec3(dy * _renderer.camera.rotationSpeed, -dx * _renderer.camera.rotationSpeed, 0.0f);

		_renderer.camera.SetRotation(updatedRot);
	}

