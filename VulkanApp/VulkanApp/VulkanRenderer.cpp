#include "VulkanRenderer.h"

#include <set>

using std::vector;
using std::set;


const vector<const char*> VulkanRenderer::validationLayers{ "VK_LAYER_KHRONOS_validation" };

#pragma region Public methods
VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* windowP)
{
	window = windowP;

	try
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
	}
	catch (const std::runtime_error& e)
	{
		printf("ERROR: %s\n", e.what());

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // This macro is equal to "VK_EXT_debug_utils"
	}

	return extensions;
}

SwapchainDetails VulkanRenderer::getSwapchainDetails(vk::PhysicalDevice device)
{
	SwapchainDetails swapchainDetails;

	// Capabilities
	swapchainDetails.surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);

	// Formats
	swapchainDetails.formats = device.getSurfaceFormatsKHR(surface);

	// Presentation modes
	swapchainDetails.presentationModes = device.getSurfacePresentModesKHR(surface);

	return swapchainDetails;
}

void VulkanRenderer::clean()
{
	mainDevice.logicalDevice.destroySwapchainKHR(swapchain);
	instance.destroySurfaceKHR(surface);

	if (enableValidationLayers) {
		destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	for (SwapchainImage& image : swapchainImages)
	{
		mainDevice.logicalDevice.destroyImageView(image.imageView);
	}

	mainDevice.logicalDevice.destroy();
	instance.destroy();
}

#pragma endregion
#pragma region Private methods
void VulkanRenderer::createInstance()
{
	//v App informations =============================================
	// Information about the application
	// This data is for developer convenience
	vk::ApplicationInfo appInfo{};
	appInfo.pApplicationName = "Vulkan App"; // Name of the app
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version of the application
	appInfo.pEngineName = "No Engine"; // Custom engine name
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Custom engine version
	appInfo.apiVersion = VK_API_VERSION_1_1; // Vulkan version (here 1.1)
	
	//^ App informations =============================================
	//v Create informations ==========================================
	// Everything we create will be created with a createInfo
	// Here, info about the vulkan creation
	vk::InstanceCreateInfo createInfo{};
	// createInfo.pNext // Extended information
	// createInfo.flags // Flags with bitfield
	createInfo.pApplicationInfo = &appInfo; // Application info from above

	// Setup extensions instance will use ===
	std::vector<const char*> instanceExtensions = getRequiredExtensions();
	//uint32_t glfwExtensionsCount = 0; // Glfw may require multiple extensions
	//const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	//for (size_t i = 0; i < glfwExtensionsCount; ++i)
	//{
	//	instanceExtensions.push_back(glfwExtensions[i]);
	//}

	// Check instance extensions
	if (!checkInstanceExtensionSupport(instanceExtensions))
	{
		throw std::runtime_error("VkInstance does not support required extensions");
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	// ======================================
	// Validation layers ====================
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}
	// ======================================
	//^ Create informations ==========================================

	// Finally create instance
	instance = vk::createInstance(createInfo);
}

void VulkanRenderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger.");
	}
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void VulkanRenderer::createSurface()
{
	// Create a surface relatively to our window
	VkSurfaceKHR _surface;

	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &_surface);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a vulkan surface.");
	}

	surface = vk::SurfaceKHR(_surface);
}

VkResult VulkanRenderer::createDebugUtilsMessengerEXT(VkInstance instance, 
													  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
													  const VkAllocationCallbacks* pAllocator, 
													  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanRenderer::createSwapchain()
{
	// We will pick best settings for the swapchain
	SwapchainDetails swapchainDetails = getSwapchainDetails(mainDevice.physicalDevice);
	vk::SurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapchainDetails.formats);
	vk::PresentModeKHR presentationMode = chooseBestPresentationMode(swapchainDetails.presentationModes);
	vk::Extent2D extent = chooseSwapExtent(swapchainDetails.surfaceCapabilities);

	//v Swapchain info ===============================================
	vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = presentationMode;
	swapchainCreateInfo.imageExtent = extent;

	// Minimal number of image in our swapchain. We will use one
	// more than the minimum to enable triple-buffering.
	uint32_t imageCount = swapchainDetails.surfaceCapabilities.minImageCount + 1;
	if (swapchainDetails.surfaceCapabilities.maxImageCount > 0 // Not limitless
		&& swapchainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapchainDetails.surfaceCapabilities.maxImageCount;
	}
	swapchainCreateInfo.minImageCount = imageCount;

	// Number of layers for each image in swapchain
	swapchainCreateInfo.imageArrayLayers = 1;
	// What attachment go with the image (e.g. depth, stencil...). Here, just color.
	swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	// Transform to perform on swapchain images
	swapchainCreateInfo.preTransform = swapchainDetails.surfaceCapabilities.currentTransform;
	// Handles blending with other windows. Here we don't blend.
	swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	// Whether to clip parts of the image not in view (e.g. when an other window overlaps)
	swapchainCreateInfo.clipped = VK_TRUE;
	//^ Swapchain info ===============================================
	//v Queue management =============================================
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);
	uint32_t queueFamilyIndices[]{ (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentationFamily };

	// If graphics and presentation families are different, share images between them
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	// When you want to pass old swapchain responsibilities when destroying it,
	// e.g. when you want to resize window, use this
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	//^ Queue management =============================================

	// Store for later use
	swapchainImageFormat = surfaceFormat.format;
	swapchainExtent = extent;

	// Create swapchain
	swapchain = mainDevice.logicalDevice.createSwapchainKHR(swapchainCreateInfo);

	//v Get swapchain's images =======================================
	vector<vk::Image> images = mainDevice.logicalDevice.getSwapchainImagesKHR(swapchain);

	for (VkImage image : images) {
		SwapchainImage swapchainImage{};
		swapchainImage.image = image;

		// Create image view
		swapchainImage.imageView = createImageView(image, swapchainImageFormat, vk::ImageAspectFlagBits::eColor);

		swapchainImages.push_back(swapchainImage);
	}
	//^ Get swapchain's images =======================================
}

void VulkanRenderer::destroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

vk::ImageView VulkanRenderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags)
{
	vk::ImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.image = image;
	viewCreateInfo.viewType = vk::ImageViewType::e2D; // Other formats can be used for cubemaps etc.
	viewCreateInfo.format = format; // Can be used for depth for instance

	// Swizzle used to remap color values. Here we keep the same.
	viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
	viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
	viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
	viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

	// Subresources allow the view to view only a part of an image
	// Here we want to see the image under the aspect of colors
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	viewCreateInfo.subresourceRange.baseMipLevel = 0; // Start mipmap level to view from
	viewCreateInfo.subresourceRange.levelCount = 1; // Number of mipmap level to view
	viewCreateInfo.subresourceRange.baseArrayLayer = 0; // Start array level to view from
	viewCreateInfo.subresourceRange.layerCount = 1; // Number of array levels to view

	// Create image view
	vk::ImageView imageView = mainDevice.logicalDevice.createImageView(viewCreateInfo);

	return imageView;
}

vk::SurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const vector<vk::SurfaceFormatKHR>& formats)
{
	// We will use RGBA 32bits normalized and SRGG non linear colorspace
	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
	{
		// All formats available by convention
		return { vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	for (auto& format : formats)
	{
		if (format.format == vk::Format::eR8G8B8A8Unorm
			&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	// Return first format if we have not our chosen format
	return formats[0];
}

vk::PresentModeKHR VulkanRenderer::chooseBestPresentationMode(const vector<vk::PresentModeKHR>& presentationModes)
{
	// We will use mail box presentation mode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == vk::PresentModeKHR::eMailbox)
		{
			return presentationMode;
		}
	}

	// Part of the Vulkan spec, so have to be available
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// Rigid extents
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	// Extents can vary
	else
	{
		// Create new extent using window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		vk::Extent2D newExtent{};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);
		// Surface also defines max and min, so make sure we are within boundaries
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
			std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
			std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

bool VulkanRenderer::checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions)
{
	// Create the vector of extensions
	vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

	// Check if given extensions are in list of available extensions
	for (const auto& checkExtension : checkExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName) == 0) // C function to check if two strings are equal 
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension) return false;
	}

	return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
	vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();

	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;

		for (const auto& extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension) return false;
	}

	return true;
}

void VulkanRenderer::getPhysicalDevice()
{
	// Get available physical devices
	vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

	// If no devices available
	if (devices.size() == 0)
	{
		throw std::runtime_error("Can't find any GPU that supports vulkan");
	}

	// Get valid device for what we want to do
	for (const auto& device : devices)
	{
		if (checkDeviceSuitable(device))
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}
}

bool VulkanRenderer::checkDeviceSuitable(vk::PhysicalDevice device)
{
	// Information about the device itself (ID, name, type, vendor, etc.)
	vk::PhysicalDeviceProperties deviceProperties = device.getProperties();

	// Information about what the device can do (geom shader, tesselation, wide lines...)
	vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

	// For now we do nothing with this info
	QueueFamilyIndices indices = getQueueFamilies(device);

	bool extensionSupported = checkDeviceExtensionSupport(device);
	bool swapchainValid = false;

	if (extensionSupported) {
		SwapchainDetails swapchainDetails = getSwapchainDetails(device);
		swapchainValid = !swapchainDetails.presentationModes.empty() && !swapchainDetails.formats.empty();
	}

	return indices.isValid() && extensionSupported && swapchainValid;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;
	vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

	// Go through each queue family and check if it has at least one required type of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Check if there is at least graphics queue
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		// Check if queue family support presentation
		VkBool32 presentationSupport = device.getSurfaceSupportKHR(static_cast<uint32_t>(indices.graphicsFamily), surface);
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}

		if (indices.isValid()) break;
		++i;
	}

	return indices;
}

void VulkanRenderer::createLogicalDevice()
{
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	// Vector for queue creation information, and set for family indices.
	// A set will only keep one indice if they are the same.
	vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };

	// Queues the logical device needs to create and info to do so.
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;
		// Vulkan needs to know how to handle multiple queues. It uses priorities.
		// 1 is the highest priority.
		queueCreateInfo.pQueuePriorities = &priority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Logical device creation
	vk::DeviceCreateInfo deviceCreateInfo{};
	// Queues info
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	// Extensions info
	// Device extensions, different from instance extensions
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	// -- Validation layers are deprecated since Vulkan 1.1
	// Features
	// For now, no device features (tessellation etc.)
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create the logical device for the given physical device
	mainDevice.logicalDevice = mainDevice.physicalDevice.createDevice(deviceCreateInfo);

	// Ensure access to queues
	graphicsQueue = mainDevice.logicalDevice.getQueue(indices.graphicsFamily, 0);
	presentationQueue = mainDevice.logicalDevice.getQueue(indices.presentationFamily, 0);
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	// Check if all of the layers in validation layers exist in the available layers
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound) return false;
	}

	return true;
}

#pragma endregion
