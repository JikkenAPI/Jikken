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
#include <array>
#include "shaderUtils.hpp"
//just temp for shader loading
#include <fstream>
#include <sstream>

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
		mCommandBuffers(),
		mSingleCmdBuffer(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mDebugCallback(VK_NULL_HANDLE),
		mAllocCallback(nullptr),
		mImageAvailableSem(VK_NULL_HANDLE),
		mRenderFinishedSem(VK_NULL_HANDLE),
		mShaderHandle(0)
	{
	}

	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
		//wait for device to be idle
		if (mDevice)
			vkDeviceWaitIdle(mDevice);

		//delete shaders
		for (auto &shader : mShaders)
		{
			for(auto &module : shader.second.modules)
				vkDestroyShaderModule(mDevice,module,mAllocCallback);
		}

		mShaders.clear();

		//destory semaphores
		if (mImageAvailableSem)
			vkDestroySemaphore(mDevice, mImageAvailableSem, mAllocCallback);
		if (mRenderFinishedSem)
			vkDestroySemaphore(mDevice, mRenderFinishedSem, mAllocCallback);

		//free command buffer
		if (mCommandBuffers.size() > 0)
		{
			vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
			mCommandBuffers.clear();
		}

		//free single command buffer
		if (mSingleCmdBuffer)
			vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mSingleCmdBuffer);

		// delete command pool
		if(mCommandPool)
			vkDestroyCommandPool(mDevice, mCommandPool, mAllocCallback);

		// destroy render pass
		if (mRenderPass)
			vkDestroyRenderPass(mDevice, mRenderPass, mAllocCallback);

		//delete color image views from the framebuffer
		for (auto &color : mSwapChainParams.colorImages)
		{
			if (color.view != VK_NULL_HANDLE)
			{
				vkDestroyImageView(mDevice, color.view, nullptr);
				color.view = VK_NULL_HANDLE;
			}
		}

		mSwapChainParams.colorImages.clear();

		//delete depth/stencil imageview
		if (mSwapChainParams.depthStencilImage.view)
		{
			vkDestroyImageView(mDevice, mSwapChainParams.depthStencilImage.view, nullptr);
			mSwapChainParams.depthStencilImage.view = nullptr;
		}

		//delete depth/stencil image
		if (mSwapChainParams.depthStencilImage.image)
		{
			vkDestroyImage(mDevice, mSwapChainParams.depthStencilImage.image, nullptr);
			mSwapChainParams.depthStencilImage.image = nullptr;
		}

		if (mSwapChainParams.swapChain)
		{
			vkDestroySwapchainKHR(mDevice, mSwapChainParams.swapChain, mAllocCallback);
		}

		//delete framebuffers
		for (auto &fb : mSwapChainParams.frameBuffers)
		{
			vkDestroyFramebuffer(mDevice, fb, mAllocCallback);
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

	bool VulkanGraphicsDevice::init(const ContextConfig &contextConfig, const NativeWindowData &windowData)
	{
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

		//get required extensions
		std::vector<const char*> requiredExtensions;
		vkutils::getRequiredInstanceExtensions(requiredExtensions);

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
		result = vkutils::createSurface(windowData, mInstance, mAllocCallback, &mSurface);
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

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

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

		//command pool
		VkCommandPoolCreateInfo cmdPoolCreateInfo;
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolCreateInfo.pNext = nullptr;
		cmdPoolCreateInfo.queueFamilyIndex = mGraphicsQueueIndex;

		result = vkCreateCommandPool(mDevice, &cmdPoolCreateInfo, mAllocCallback, &mCommandPool);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateCommandPool failed\n");
			return false;
		}

		//single command buffer - used to execute single commands
		VkCommandBufferAllocateInfo cmdBufAllocInfo = {};		
		cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocInfo.pNext = nullptr;
		cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocInfo.commandPool = mCommandPool;
		cmdBufAllocInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(mDevice, &cmdBufAllocInfo, &mSingleCmdBuffer);

		if (result != VK_SUCCESS)
		{
			std::printf("vkAllocateCommandBuffers failed\n");
			return false;
		}

		//setup swapchain and framebuffers
		if (!_createSwapchain())
		{
			std::printf("Failed to create swapchain\n");
			return false;
		}

		//create default renderpass
		if (!_createDefaultRenderPass())
		{
			std::printf("Failed to create default renderpass");
			return false;
		}

		//create framebuffers
		if (!_createFramebuffers())
		{
			std::printf("Failed to create framebuffers");
			return false;
		}

		//regular command buffers - one per swapchain color image
		mCommandBuffers.resize(mSwapChainParams.colorImages.size());
		cmdBufAllocInfo.commandBufferCount = static_cast<uint32_t>(mSwapChainParams.colorImages.size());

		result = vkAllocateCommandBuffers(mDevice, &cmdBufAllocInfo, mCommandBuffers.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkAllocateCommandBuffers failed\n");
			return false;
		}

		//create semaphores
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		//Image available sempahore
		result = vkCreateSemaphore(mDevice, &semaphoreCreateInfo, mAllocCallback, &mImageAvailableSem);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateSemaphore failed\n");
			return false;
		}
		//Rendering finished semaphore
		result = vkCreateSemaphore(mDevice, &semaphoreCreateInfo, mAllocCallback, &mRenderFinishedSem);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateSemaphore failed\n");
			return false;
		}


		return true;
	}

	bool VulkanGraphicsDevice::_createDefaultRenderPass()
	{
		//render pass for default framebuffers
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.flags = 0;
		colorAttachment.format = mSwapChainParams.colorFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.flags = 0;
		depthAttachment.format = mSwapChainParams.depthStencilFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(mDevice, &renderPassInfo, mAllocCallback, &mRenderPass);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateRenderPass failed\n");
			return false;
		}

		return true;
	}

	bool VulkanGraphicsDevice::_createFramebuffers()
	{
		//delete framebuffers
		for (auto &fb : mSwapChainParams.frameBuffers)
		{
			vkDestroyFramebuffer(mDevice, fb, mAllocCallback);
		}

		mSwapChainParams.frameBuffers.clear();

		size_t imageCount = mSwapChainParams.colorImages.size();
		mSwapChainParams.frameBuffers.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			std::array<VkImageView, 2> attachments = {
				mSwapChainParams.colorImages[i].view,
				mSwapChainParams.depthStencilImage.view
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = mRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = mSwapChainParams.extent.width;
			framebufferInfo.height = mSwapChainParams.extent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainParams.frameBuffers[i]);

			if (result != VK_SUCCESS)
			{
				std::printf("vkCreateFramebuffer failed\n");
				return false;
			}
		}

		return true;
	}

	bool VulkanGraphicsDevice::_createSwapchain()
	{
		//delete color image views from the framebuffer
		for (auto &color : mSwapChainParams.colorImages)
		{
			if (color.view != VK_NULL_HANDLE)
			{
				vkDestroyImageView(mDevice, color.view, nullptr);
				color.view = VK_NULL_HANDLE;
			}
		}

		mSwapChainParams.colorImages.clear();

		//delete depth/stencil imageview
		if (mSwapChainParams.depthStencilImage.view)
		{
			vkDestroyImageView(mDevice, mSwapChainParams.depthStencilImage.view, nullptr);
			mSwapChainParams.depthStencilImage.view = nullptr;
		}

		//delete depth/stencil image
		if (mSwapChainParams.depthStencilImage.image)
		{
			vkDestroyImage(mDevice, mSwapChainParams.depthStencilImage.image, nullptr);
			mSwapChainParams.depthStencilImage.image = nullptr;
		}

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

		mSwapChainParams.colorFormat = desiredFormat.format;
		mSwapChainParams.depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;//todo format passed in from client
		mSwapChainParams.extent = createInfo.imageExtent;

		//color images
		uint32_t imageCount = 0;
		result = vkGetSwapchainImagesKHR(mDevice, mSwapChainParams.swapChain, &imageCount, nullptr);
		if (result != VK_SUCCESS || imageCount == 0)
		{
			std::printf("vkGetSwapchainImagesKHR failed\n");
			return false;
		}

		mSwapChainParams.colorImages.resize(imageCount);

		std::vector<VkImage> images(imageCount);
		result = vkGetSwapchainImagesKHR(mDevice, mSwapChainParams.swapChain, &imageCount, images.data());
		if (result != VK_SUCCESS)
		{
			std::printf("vkGetSwapchainImagesKHR failed\n");
			return false;
		}

		VkComponentMapping mapping = { VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY };
		VkImageSubresourceRange colorRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
		for (uint32_t i = 0; i < imageCount; i++)
		{
			mSwapChainParams.colorImages[i].image = images[i];
			mSwapChainParams.colorImages[i].extent = mSwapChainParams.extent;

			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.image = images[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = mSwapChainParams.colorFormat;
			imageViewCreateInfo.components = mapping;
			imageViewCreateInfo.subresourceRange = colorRange;

			result = vkCreateImageView(mDevice, &imageViewCreateInfo, mAllocCallback, &mSwapChainParams.colorImages[i].view);
			if (result != VK_SUCCESS)
			{
				std::printf("vkCreateImageView failed\n");
				return false;
			}
		}

		//depth/stencil - only a single one
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = mSwapChainParams.depthStencilFormat;
		imageCreateInfo.extent.width = mSwapChainParams.extent.width;
		imageCreateInfo.extent.height = mSwapChainParams.extent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		result = vkCreateImage(mDevice, &imageCreateInfo, mAllocCallback, &mSwapChainParams.depthStencilImage.image);

		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateImage failed for depth/stencil creation\n");
			return false;
		}

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(mDevice, mSwapChainParams.depthStencilImage.image, &memReq);

		uint32_t memType = vkutils::findMemoryType(mPhysicalDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (memType == UINT32_MAX)
		{
			std::printf("Could not find valid memory type for depth/stencil\n");
			return false;
		}

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = memType;
		allocInfo.pNext = nullptr;

		result = vkAllocateMemory(mDevice, &allocInfo, mAllocCallback, &mSwapChainParams.depthStencilImage.memory);

		if (result != VK_SUCCESS)
		{
			std::printf("vkAllocateMemory failed for depth/stencil creation\n");
			return false;
		}

		result = vkBindImageMemory(mDevice, mSwapChainParams.depthStencilImage.image, mSwapChainParams.depthStencilImage.memory, 0);

		if (result != VK_SUCCESS)
		{
			std::printf("vkBindImageMemory failed for depth/stencil creation\n");
			return false;
		}

		VkImageSubresourceRange depthRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,0,1,0,1 };
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = mSwapChainParams.depthStencilImage.image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSwapChainParams.depthStencilFormat;
		imageViewCreateInfo.components = mapping;
		imageViewCreateInfo.subresourceRange = depthRange;

		result = vkCreateImageView(mDevice, &imageViewCreateInfo, mAllocCallback, &mSwapChainParams.depthStencilImage.view);
		if (result != VK_SUCCESS)
		{
			std::printf("vkCreateImageView failed for depth/stencil creation\n");
			return false;
		}

		mSwapChainParams.depthStencilImage.extent = mSwapChainParams.extent;

		//viewport params
		mViewPortParams.viewport.height = static_cast<float>(mSwapChainParams.extent.height);
		mViewPortParams.viewport.width = static_cast<float>(mSwapChainParams.extent.width);
		mViewPortParams.viewport.x = 0.f;
		mViewPortParams.viewport.y = 0.f;
		mViewPortParams.viewport.minDepth = 0.f;
		mViewPortParams.viewport.maxDepth = 1.0f;
		mViewPortParams.scissor.offset = { 0, 0 };
		mViewPortParams.scissor.extent = mSwapChainParams.extent;

		//Store present info struct, only thing that changes is the pImageIndices per frame but that is stored as a pointer so it gets the new value as it changes
		mPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		mPresentInfo.pNext = nullptr;
		mPresentInfo.swapchainCount = 1;
		mPresentInfo.pSwapchains = &mSwapChainParams.swapChain;
		mPresentInfo.waitSemaphoreCount = 1;
		mPresentInfo.pWaitSemaphores = &mRenderFinishedSem;
		mPresentInfo.pResults = nullptr;

		return true;
	}

	ShaderHandle VulkanGraphicsDevice::createShader(const std::vector<ShaderDetails> &shaders)
	{
		//check if this will put us over the maximum number of shader handles
		if ((mShaderHandle + 1) == InvalidHandle)
		{
			std::printf("Too many shader handles");
			return InvalidHandle;
		}

		VulkanShader shader;

		for (const ShaderDetails &details : shaders)
		{
			//file loading is only temporary
			std::ifstream stream(details.file);
			if (!stream.is_open())
			{
				std::printf("Unable to open %s for reading! Aborting!", details.file.c_str());
				return InvalidHandle;
			}

			std::stringstream buffer;
			buffer << stream.rdbuf();

			std::vector<uint32_t> spirvData;
			if (!ShaderUtils::convertGlslToSpirv(details.stage, buffer.str(), spirvData))
			{
				std::printf("Unable to open convert %s to spirv format! Aborting!", details.file.c_str());
				return InvalidHandle;
			}
			
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = spirvData.size() * sizeof(uint32_t);
			createInfo.pCode = spirvData.data();

			VkShaderModule module;
			VkResult result = vkCreateShaderModule(mDevice, &createInfo, mAllocCallback, &module);
			if (result != VK_SUCCESS)
			{
				std::printf("vkCreateShaderModule failed\n");
				return InvalidHandle;
			}

			shader.modules.push_back(module);

			VkPipelineShaderStageCreateInfo piplineStage;
			piplineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			piplineStage.pSpecializationInfo = nullptr;
			piplineStage.flags = 0;
			piplineStage.pNext = nullptr;
			piplineStage.module = module;
			piplineStage.pName = "main";
			piplineStage.stage = vkutils::getShaderStageFlag(details.stage);

			shader.stages.push_back(piplineStage);
		}

		ShaderHandle handle = mShaderHandle++;
		mShaders[handle] = { shader };
		return handle;
	}

	BufferHandle VulkanGraphicsDevice::createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, void *data)
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
	
	void VulkanGraphicsDevice::presentFrame()
	{
		vkQueueWaitIdle(mGraphicsQueue);
		VkResult result = vkQueuePresentKHR(mGraphicsQueue, &mPresentInfo);

		switch (result)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			break; //TODO: handle re-creating swapchain,mostly likely unhandled window resize
		default:
			std::printf("Problem occurred during image presentation\n");
			break;
		}
	}

	//Commands
	void VulkanGraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{
	}

	void VulkanGraphicsDevice::_beginFrameCmd(BeginFrameCommand *cmd)
	{
		VkClearValue clearValue[2];
		bool clearDepthStencil = false;
		if (cmd->clearFlag & ClearBufferFlags::eColor)
		{
			clearValue[0].color = { {cmd->clearColor[0], cmd->clearColor[1], cmd->clearColor[2], cmd->clearColor[3]} };
		}

		if (cmd->clearFlag & ClearBufferFlags::eDepth)
		{
			clearValue[1].depthStencil.depth = cmd->depth;
			//we give stencil a default value of 0
			clearValue[1].depthStencil.stencil = 0;
			clearDepthStencil = true;
		}

		if (cmd->clearFlag & ClearBufferFlags::eStencil)
		{
			//give depth a default value
			if (!clearDepthStencil)
				clearValue[1].depthStencil.depth = 1.0f;

			clearValue[1].depthStencil.stencil = cmd->stencil;
			clearDepthStencil = true;
		}

		//get the next image in the swap chain
		//todo check result for VK_ERROR_OUT_OF_DATE_KHR and if found we need to rebuild the swapchain as it will be out of date (possibly a window resize missed)
		VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChainParams.swapChain, UINT64_MAX, mImageAvailableSem, VK_NULL_HANDLE, &mSwapChainParams.currentImageIndex);

		//set image index for present info
		mPresentInfo.pImageIndices = &mSwapChainParams.currentImageIndex;

		VkCommandBuffer cmdBuffer = mCommandBuffers[mSwapChainParams.currentImageIndex];

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = mRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = mSwapChainParams.extent.width;
		renderPassBeginInfo.renderArea.extent.height = mSwapChainParams.extent.height;
		renderPassBeginInfo.clearValueCount = clearDepthStencil ? 2 : 1;
		renderPassBeginInfo.pClearValues = clearValue;
		renderPassBeginInfo.framebuffer = mSwapChainParams.frameBuffers[mSwapChainParams.currentImageIndex];

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(cmdBuffer);
		vkEndCommandBuffer(cmdBuffer);

		// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		// The submit info structure specifices a command buffer queue submission batch
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.pWaitSemaphores = &mImageAvailableSem;
		submitInfo.waitSemaphoreCount = 1;																		
		submitInfo.pSignalSemaphores = &mRenderFinishedSem;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.commandBufferCount = 1;
		
		result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			std::printf("vkQueueSubmit failed\n");			
		}
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

	//we have to fake this as VAO is an opengl only concept
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