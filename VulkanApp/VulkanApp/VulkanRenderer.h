#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "VulkanUtilities.h"


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

	int init(GLFWwindow* windowP); // <------------------------------- INIT 

	std::vector<const char*> getRequiredExtensions();
	SwapchainDetails getSwapchainDetails(vk::PhysicalDevice device);

	void draw(); // <------------------------------------------------- DRAW 

	void clean(); // <------------------------------------------------ CLEAN 

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
	vk::Queue presentationQueue;

	int currentFrame{ 0 };
	const int MAX_FRAME_DRAWS{ 2 }; // <--- Should be less than the nb of swapchain images, which is 3
	std::vector<vk::Fence> drawFences;

	// -- SURFACE --
	vk::SurfaceKHR surface;
	void createSurface();

	//v Swapchain ====================================================
	vk::SwapchainKHR swapchain;

	vk::Format swapchainImageFormat;
	vk::Extent2D swapchainExtent;

	std::vector<SwapchainImage> swapchainImages;
	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags);

	void createSwapchain();
	
	vk::SurfaceFormatKHR chooseBestSurfaceFormat(const vector<vk::SurfaceFormatKHR>& formats);
	vk::PresentModeKHR chooseBestPresentationMode(const vector<vk::PresentModeKHR>& presentationModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);
	//^ Swapchain ====================================================
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

	//v Graphic Pipeline =============================================
	vk::RenderPass renderPass;
	void createRenderPass();
	
	vk::PipelineLayout pipelineLayout;

	// -- GRAPHICS PIPELINE --
	vk::Pipeline graphicsPipeline;
	void createGraphicPipeline();
	VkShaderModule createShaderModule(const vector<char>& code);

	// -- FRAMEBUFFER --
	std::vector<vk::Framebuffer> swapchainFramebuffers;
	void createFramebuffers();

	// -- COMMAND POOL --
	vk::CommandPool graphicsCommandPool;
	void createGraphicsCommandPool();

	// -- COMMAND BUFFER --
	void recordCommands();
	std::vector<vk::CommandBuffer> commandBuffers;
	void createGraphicsCommandBuffers();

	//^ Graphic Pipeline =============================================

	//v Synchronisation ==============================================
	std::vector<vk::Semaphore> imageAvailable;
	std::vector<vk::Semaphore> renderFinished;

	void createSynchronisation();

	//^ Synchronisation ==============================================
};
