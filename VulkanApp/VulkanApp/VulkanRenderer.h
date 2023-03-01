#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <stdexcept>
#include <vector>

#include "VulkanUtilies.h"

struct 
{
	vk::PhysicalDevice physicalDevice;
	vk::Device logicalDevice;
} mainDevice;

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	int init(GLFWwindow* windowP);

	std::vector<const char*> getRequiredExtensions();

	void clean();

#ifdef NODEBUG
	static const bool enableValidationLayers = false;
#else
	static const bool enableValidationLayers = true;
#endif
	static const std::vector<const char*> validationLayers;

private:
	void createInstance();

	GLFWwindow* window;
	vk::Instance instance; // vk:: -> C++ API

	vk::Queue graphicsQueue;

	vk::SurfaceKHR surface;
	vk::Queue presentationQueue;

	vk::SurfaceKHR createSurface();

	//v Messenger ====================================================
	VkDebugUtilsMessengerEXT debugMessenger;
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult createDebugUtilsMessengerEXT(VkInstance instance,
										  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
										  const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger
	);
	void destroyDebugUtilsMessengerEXT(VkInstance instance,
									   VkDebugUtilsMessengerEXT debugMessenger, 
									   const VkAllocationCallbacks* pAllocator
	);
	//^ Messenger ====================================================

	/// Enumerate physical devices and store them
	/// Then check if the devices are suitable for future uses
	void getPhysicalDevice();
	/// 
	void createLogicalDevice();

	QueueFamilyIndices getQueueFamilies(vk::PhysicalDevice device);

	//v Various checks ===============================================
	bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);
	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
	/// Check if the devices are suitable for future uses by gathering the info about the physical device
	/// and getting its queue family indices. If they are valid, it returns true
	bool checkDeviceSuitable(vk::PhysicalDevice device);
	bool checkValidationLayerSupport();
	//^ Various checks ===============================================
};
