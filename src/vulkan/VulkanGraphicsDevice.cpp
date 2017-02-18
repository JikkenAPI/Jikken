//-----------------------------------------------------------------------------
// Jikken - 3D Abstract High Performance Graphics API
// Copyright(c) 2017 Jeff Hutchinson
// Copyright(c) 2017 Tim Barnes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#include <cassert>
#include <cstdlib>
#include "vulkan/VulkanGraphicsDevice.hpp"
#include "vulkan/VulkanUtil.hpp"
//just temp
#include <GLFW/glfw3.h>

namespace Jikken
{
	VulkanGraphicsDevice::VulkanGraphicsDevice() :
		mInstance(VK_NULL_HANDLE),
		mSurface(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mGraphicsQueue(VK_NULL_HANDLE),
		mComputeQueue(VK_NULL_HANDLE),
		mGraphicsQueueIndex(UINT32_MAX),
		mComputeQueueIndex(UINT32_MAX),
		mRenderPass(VK_NULL_HANDLE),
		mDebugCallback(VK_NULL_HANDLE),
		mAllocCallback(nullptr)
	{
	}

	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
		//wait for device to be idle
		if (mDevice)
			vkDeviceWaitIdle(mDevice);

		// destroy render pass
		if (mRenderPass)
			vkDestroyRenderPass(mDevice, mRenderPass, mAllocCallback);

		//cleanup swapchain
		for (auto &img : mSwapChainParams.images)
		{
			if (img.view != VK_NULL_HANDLE)
			{
				vkDestroyImageView(mDevice, img.view, nullptr);
				img.view = VK_NULL_HANDLE;
			}
		}

		mSwapChainParams.images.clear();

		if (mSwapChainParams.swapChain)
		{
			vkDestroySwapchainKHR(mDevice, mSwapChainParams.swapChain, mAllocCallback);
		}

		// destroy logical device
		if (mDevice)
			vkDestroyDevice(mDevice, mAllocCallback);

		if (mSurface)
			vkDestroySurfaceKHR(mInstance, mSurface, mAllocCallback);

		if (mDebugCallback)
		{
			auto debugReportDestroyFunc = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT");
			if (debugReportDestroyFunc != nullptr)
			{
				debugReportDestroyFunc(mInstance, mDebugCallback, mAllocCallback);
			}
		}

		if (mInstance)
			vkDestroyInstance(mInstance, mAllocCallback);
	}

	bool VulkanGraphicsDevice::init(void *glfwWinHandle)
	{
		if (!glfwVulkanSupported())
		{
			std::printf("Vulkan is not supported\n");
			return false;
		}

		//validation layers
	#ifdef _DEBUG
		bool validationLayersEnabled = true;
	#else
		bool validationLayersEnabled = false;
	#endif

		//grab vulkan instance extension list
		uint32_t extensionsCount = 0;
		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
		if (result != VK_SUCCESS || extensionsCount == 0)
		{
			std::printf("vkEnumerateInstanceExtensionProperties failed\n");
			return false;
		}

		std::vector<VkExtensionProperties> instanceExtensionList(extensionsCount);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, instanceExtensionList.data());

		if (result != VK_SUCCESS)
		{
			std::printf("vkEnumerateInstanceExtensionProperties failed\n");
			return false;
		}

		//get required extensions from glfw
		uint32_t glfwExtCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		if (glfwExtCount == 0)
		{
			std::printf("Failed to find any required extensions\n");
			return false;
		}

		std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtCount);

		if (validationLayersEnabled)
			requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		// check required extension list against all availble extensions
		for (const auto &ext : requiredExtensions)
		{
			if (!vkutils::checkExtension(ext, instanceExtensionList))
			{
				std::printf("Required extension: %s not found\n", ext);
				return false;
			}
		}

		//setup validation layers - VK_LAYER_LUNARG_standard_validation enables:
		//VK_LAYER_GOOGLE_threading, VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_image, VK_LAYER_LUNARG_core_validation,VK_LAYER_LUNARG_swapchain, and VK_LAYER_GOOGLE_unique_objects
		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

		uint32_t layerCount = 0;

		if (validationLayersEnabled)
		{
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const auto &layer : validationLayers)
			{
				//this is not fatal so just disable validation layers if it is not found
				if (!vkutils::checkLayer(layer, availableLayers))
				{
					std::printf("Could not find validation layer: %s Disabling validation layers\n", layer);
					validationLayersEnabled = false;
				}
			}
		}

		//setup application and instance info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Jikken";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Jikken";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		if (validationLayersEnabled)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
			createInfo.enabledLayerCount = 0;

		//finally create vulkan instance
		result = vkCreateInstance(&createInfo, mAllocCallback, &mInstance);

		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateInstance failed\n");
			return false;
		}

		//setup debug callback if validation layers enabled
		if (validationLayersEnabled)
		{
			VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debugCreateInfo.pfnCallback = vkutils::debugCallback;

			auto debugReportFunc = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
			if (debugReportFunc != nullptr)
			{
				result = debugReportFunc(mInstance, &debugCreateInfo, mAllocCallback, &mDebugCallback);

				if (result != VK_SUCCESS)
				{
					std::printf("vkCreateDebugReportCallbackEXT failed\n");
					return false;
				}
			}
			else
			{
				std::printf("Failed to retrieve vkCreateDebugReportCallbackEXT address. Debug callbacks will be disabled\n");
				validationLayersEnabled = false;
			}
		}

		//create window surface
		result = glfwCreateWindowSurface(mInstance, static_cast<GLFWwindow*>(glfwWinHandle), mAllocCallback, &mSurface);
		if (result != VK_SUCCESS)
		{
			std::printf("Failed to create vulkan window surface\n");
			return false;
		}

		//find physical devices
		uint32_t numDevices = 0;
		result = vkEnumeratePhysicalDevices(mInstance, &numDevices, nullptr);
		if (result != VK_SUCCESS || numDevices == 0)
		{
			std::printf("vkEnumeratePhysicalDevices failed\n");
			return false;
		}

		std::vector<VkPhysicalDevice> physicalDevices(numDevices);
		result = vkEnumeratePhysicalDevices(mInstance, &numDevices, physicalDevices.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkEnumeratePhysicalDevices failed\n");
			return false;
		}

		//get our queue index and hopefully find an acceptable device
		for (const auto &device : physicalDevices)
		{
			//break on first acceptable device
			if (vkutils::checkPhysicalDevice(device, mSurface, mGraphicsQueueIndex,mComputeQueueIndex))
			{
				mPhysicalDevice = device;
				break;
			}
		}

		if (mGraphicsQueueIndex == UINT32_MAX || mComputeQueueIndex == UINT32_MAX)
		{
			std::printf("Failed to find suitable graphics or compute queue\n");
			return false;
		}

		if (mPhysicalDevice == VK_NULL_HANDLE)
		{
			std::printf("Failed to select a physical device\n");
			return false;
		}

		//print some information about our physical device
		vkutils::printDeviceInfo(mPhysicalDevice);

		//create logical device
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = mGraphicsQueueIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);

		//need swap chain extension - this extension is already checked above with checkPhysicalDevice, no need to check again
		std::vector<const char*> requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

		deviceCreateInfo.enabledLayerCount = 0;
		if (validationLayersEnabled)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}

		//create vulkan device
		//todo: for some reason this is creating vkCreateSampler errors here, find out why because we are not calling that function.
		//happens on both intel/nv drivers and also happens with lunarg demo, possible lunarg debug layer bug??
		result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, mAllocCallback, &mDevice);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateDevice failed\n");
			return false;
		}

		//grab graphics queue handle
		vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);
		//grab compute queue handle
		vkGetDeviceQueue(mDevice, mComputeQueueIndex, 0, &mComputeQueue);

		if (!mGraphicsQueue || !mComputeQueue)
		{
			std::printf("Failed to retrieve graphics or compute queue\n");
			return false;
		}

		//create swapchain
		if (!_createSwapchain())
		{
			std::printf("Failed to create vulkan swapchain\n");
			return false;
		}

		//setup backbuffer and render pass
		VkAttachmentDescription backBufAttachmentDesc[2];
		//color
		backBufAttachmentDesc[0].flags = 0;
		backBufAttachmentDesc[0].format = mSwapChainParams.format;
		backBufAttachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
		backBufAttachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		backBufAttachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		backBufAttachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		backBufAttachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		backBufAttachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		backBufAttachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//depth/stencil
		backBufAttachmentDesc[1].flags = 0;
		backBufAttachmentDesc[1].format = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT_S8_UINT
		backBufAttachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
		backBufAttachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		backBufAttachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		backBufAttachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		backBufAttachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		backBufAttachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		backBufAttachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//attachment reference
		VkAttachmentReference attacmentRefs[3];
		//color
		attacmentRefs[0].attachment = 0;
		attacmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//resolve
		attacmentRefs[1].attachment = VK_ATTACHMENT_UNUSED;
		attacmentRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//depth/stencil
		attacmentRefs[2].attachment = 1;
		attacmentRefs[2].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc;
		subpassDesc.flags = 0;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.inputAttachmentCount = 0;
		subpassDesc.pInputAttachments = VK_NULL_HANDLE;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &attacmentRefs[0];
		subpassDesc.pResolveAttachments = &attacmentRefs[1];
		subpassDesc.pDepthStencilAttachment = &attacmentRefs[2];
		subpassDesc.preserveAttachmentCount = 0;
		subpassDesc.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo renderPassInfo;
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = backBufAttachmentDesc;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDesc;
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = VK_NULL_HANDLE;

		result = vkCreateRenderPass(mDevice, &renderPassInfo, mAllocCallback, &mRenderPass);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateRenderPass failed\n");
			return false;
		}

		return true;
	}

	bool VulkanGraphicsDevice::_createSwapchain()
	{
		//delete image views
		for (auto &img : mSwapChainParams.images)
		{
			if (img.view != VK_NULL_HANDLE)
			{
				vkDestroyImageView(mDevice, img.view, nullptr);
				img.view = VK_NULL_HANDLE;
			}
		}

		mSwapChainParams.images.clear();

		VkSurfaceCapabilitiesKHR surfaceCaps;
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &surfaceCaps);
		if (result != VK_SUCCESS)
		{
			std::printf("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed\n");
			return false;
		}

		uint32_t formatCount;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
		if (result != VK_SUCCESS || formatCount == 0)
		{
			std::printf("vkGetPhysicalDeviceSurfaceFormatsKHR failed\n");
			return false;
		}

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, surfaceFormats.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkGetPhysicalDeviceSurfaceFormatsKHR failed\n");
			return false;
		}

		uint32_t presentCount;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentCount, nullptr);
		if (result != VK_SUCCESS || presentCount == 0)
		{
			std::printf("vkGetPhysicalDeviceSurfacePresentModesKHR failed\n");
			return false;
		}

		std::vector<VkPresentModeKHR> presentModes(presentCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentCount, presentModes.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkGetPhysicalDeviceSurfacePresentModesKHR failed\n");
			return false;
		}

		uint32_t swapImageCount = vkutils::getSwapChainNumImages(surfaceCaps);
		VkSurfaceFormatKHR desiredFormat = vkutils::getSwapChainFormat(surfaceFormats);
		VkImageUsageFlags desiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSurfaceTransformFlagBitsKHR desiredTransform = vkutils::getSwapChainTransform(surfaceCaps);
		VkPresentModeKHR desiredPresentMode = vkutils::getSwapChainPresentMode(presentModes);
		VkSwapchainKHR oldSwapChain = mSwapChainParams.swapChain;

		if (static_cast<int32_t>(desiredUsage) == -1)
			return false;

		if (static_cast<int32_t>(desiredPresentMode) == -1)
			return false;

		//swapchain create info
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = mSurface;
		createInfo.minImageCount = swapImageCount;
		createInfo.imageFormat = desiredFormat.format;
		createInfo.imageColorSpace = desiredFormat.colorSpace;
		createInfo.imageExtent = vkutils::chooseSwapExtent(surfaceCaps);
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = desiredUsage;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = desiredTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = desiredPresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = oldSwapChain;

		result = vkCreateSwapchainKHR(mDevice, &createInfo, mAllocCallback, &mSwapChainParams.swapChain);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateSwapchainKHR failed\n");
			return false;
		}

		if (oldSwapChain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(mDevice, mSwapChainParams.swapChain, mAllocCallback);

		mSwapChainParams.format = desiredFormat.format;
		mSwapChainParams.extent = createInfo.imageExtent;

		//images
		uint32_t imageCount = 0;
		result = vkGetSwapchainImagesKHR(mDevice, mSwapChainParams.swapChain, &imageCount, nullptr);
		if (result != VK_SUCCESS || imageCount == 0)
		{
			std::printf("vkGetSwapchainImagesKHR failed\n");
			return false;
		}

		mSwapChainParams.images.resize(imageCount);

		std::vector<VkImage> images(imageCount);
		result = vkGetSwapchainImagesKHR(mDevice, mSwapChainParams.swapChain, &imageCount, images.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkGetSwapchainImagesKHR failed\n");
			return false;
		}

		VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
		VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
		for (uint32_t i = 0; i < imageCount; i++)
		{
			mSwapChainParams.images[i].image = images[i];

			VkImageViewCreateInfo imageCreateInfo = {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;
			imageCreateInfo.flags = 0;
			imageCreateInfo.image = mSwapChainParams.images[i].image;
			imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageCreateInfo.format = mSwapChainParams.format;
			imageCreateInfo.components = mapping;
			imageCreateInfo.subresourceRange = range;

			result = vkCreateImageView(mDevice, &imageCreateInfo, mAllocCallback, &mSwapChainParams.images[i].view);
			if (result != VK_SUCCESS)
			{
				std::printf("vkCreateImageView failed\n");
				return false;
			}
		}

		//viewport params
		mViewPortParams.viewport.height = static_cast<float>(mSwapChainParams.extent.height);
		mViewPortParams.viewport.width = static_cast<float>(mSwapChainParams.extent.width);
		mViewPortParams.viewport.x = 0.f;
		mViewPortParams.viewport.y = 0.f;
		mViewPortParams.viewport.minDepth = 0.f;
		mViewPortParams.viewport.maxDepth = 1.0f;
		mViewPortParams.scissor.offset = { 0, 0 };
		mViewPortParams.scissor.extent = mSwapChainParams.extent;

		return true;
	}

	ShaderHandle VulkanGraphicsDevice::createShader(const std::vector<ShaderDetails> &shaders)
	{
		return InvalidHandle;
	}

	BufferHandle VulkanGraphicsDevice::createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, float *data)
	{
		return InvalidHandle;
	}

	LayoutHandle VulkanGraphicsDevice::createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{
		return InvalidHandle;
	}

	VertexArrayHandle VulkanGraphicsDevice::createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer)
	{
		return InvalidHandle;
	}

	void VulkanGraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index)
	{
	}

	void VulkanGraphicsDevice::deleteVertexInputLayout(LayoutHandle handle)
	{
	}

	void VulkanGraphicsDevice::deleteVAO(VertexArrayHandle handle)
	{
	}

	void VulkanGraphicsDevice::deleteBuffer(BufferHandle handle)
	{
	}

	void VulkanGraphicsDevice::deleteShader(ShaderHandle handle)
	{
	}

	void VulkanGraphicsDevice::present()
	{
	}

	//Commands
	void VulkanGraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_reallocBufferCmd(ReallocBufferCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_drawCmd(DrawCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_drawInstanceCmd(DrawInstanceCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_clearBufferCmd(ClearBufferCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_bindVAOCmd(BindVAOCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_viewportCmd(ViewportCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_blendStateCmd(BlendStateCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_depthStencilStateCmd(DepthStencilStateCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_cullStateCmd(CullStateCommand *cmd)
	{
	}
}