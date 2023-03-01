#pragma once

#include <vector>

/// Store indices (locations) of queue families and check if each family is valid
struct QueueFamilyIndices {
	int graphicsFamily = -1; // Location of Graphics Queue Family
	int presentationFamily = -1; // Location of Presentation Queue Family

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

const std::vector<const char*> deviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};