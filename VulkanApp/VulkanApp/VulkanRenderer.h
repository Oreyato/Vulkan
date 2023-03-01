#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <stdexcept>
#include <vector>

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

};

