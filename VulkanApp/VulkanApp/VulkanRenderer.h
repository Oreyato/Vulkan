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

	void clean();

private:
	GLFWwindow* window;
	vk::Instance instance; // vk:: -> C++ API

	void createInstance();
	bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);

	/// Enumerate physical devices and store them
	/// Then check if the devices are suitable for future uses
	void getPhysicalDevice();
	/// Check if the devices are suitable for future uses by gathering the info about the physical device
	/// and getting its queue family indices. If they are valid, it returns true
	bool checkDeviceSuitable(vk::PhysicalDevice device);
	QueueFamilyIndices getQueueFamilies(vk::PhysicalDevice device);
};