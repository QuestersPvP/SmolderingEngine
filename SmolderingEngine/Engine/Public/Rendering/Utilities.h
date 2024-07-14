#pragma once

#include <fstream>

const std::vector<const char*> DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::vector<char> ReadFile(const std::string& InFileName)
{
	// std::ios::ate - start reading from end of file
	std::ifstream File(InFileName, std::ios::binary | std::ios::ate);

	if (!File.is_open())
		throw std::runtime_error("Failed to open file!");

	// get the size of the file
	size_t FileSize = (size_t)File.tellg();
	std::vector<char> FileBuffer(FileSize);

	// move to start of file
	File.seekg(0);

	File.read(FileBuffer.data(), FileSize);

	File.close();

	return FileBuffer;
}

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

struct SwapchainDetails
{
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;		// Surface properties
	std::vector<VkSurfaceFormatKHR> SurfaceFormats;		// Surface image formats
	std::vector<VkPresentModeKHR> PresentationModes;	// How images should be presented
};

struct SwapchainImage
{
	VkImage Image;
	VkImageView ImageView;
};
