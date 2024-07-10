#pragma once

const std::vector<const char*> DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Indicies (locations) of Queue families
struct QueueFamilyIndicies
{
	int GraphicsFamily = -1;		// Location of Graphics Queue Family
	int PresentationFamily = -1;	// Location of Presentation Queue Family

	/* Check if the Queue Family is valid */
	bool IsValid()
	{
		return GraphicsFamily >= 0 && PresentationFamily >= 0;
	}
};
