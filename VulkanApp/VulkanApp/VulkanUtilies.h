#pragma once

#include <vector>
#include <iostream>

#include <vulkan/vulkan.hpp>

using std::vector;


/// Store indices (locations) of queue families and check if each family is valid
struct QueueFamilyIndices {
	int graphicsFamily = -1; // Location of Graphics Queue Family
	int presentationFamily = -1; // Location of Presentation Queue Family

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

// Extensions to support
const std::vector<const char*> deviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/// <param name="messageSeverity"></param>
/// <param name="messageType"></param>
/// <param name="pCallbackData">: refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message itself</param>
/// <param name="pUserData">: contains a pointer that was specified during the setup of the callback and allows to pass user own data to it</param>
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT messageType,
													const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
													void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
};

// Use to store swapchains
struct SwapchainDetails {
	// What the surface is capable of displaying, e.g. image size/extent
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;

	// Vector of the image formats, e.g. RGBA
	vector<vk::SurfaceFormatKHR> formats;

	// Vector of presentation modes
	vector<vk::PresentModeKHR> presentationModes;
};