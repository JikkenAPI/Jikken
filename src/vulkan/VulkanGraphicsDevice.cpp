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
		mGraphicsQueueIndex(UINT32_MAX),
		mDebugCallback(VK_NULL_HANDLE),
		mAllocCallback(nullptr)
	{
	}

	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
		//wait for device to be idle
		if (mDevice)
			vkDeviceWaitIdle(mDevice);

		// cleanup other stuff

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
		//	return false;
		}

		//validation layers
	#ifdef _DEBUG
		bool validationLayersEnabled = true;
	#else
		bool validationLayersEnabled = false
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

		//setup validation layers - just VK_LAYER_LUNARG_standard_validation for now. There are plenty more though ;)
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
			if (vkutils::checkPhysicalDevice(device, mSurface, mGraphicsQueueIndex))
			{
				mPhysicalDevice = device;
				break;
			}
		}

		if (mGraphicsQueueIndex == UINT32_MAX)
		{
			std::printf("Failed to find suitable graphics queue\n");
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

		if (!mGraphicsQueue)
		{
			std::printf("Failed to retrieve graphics queue\n");
			return false;
		}

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

	void VulkanGraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, int32_t index)
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

	void VulkanGraphicsDevice::submitCommandQueue(CommandQueue *cmdQueue)
	{
	}

	void VulkanGraphicsDevice::present()
	{
	}
}