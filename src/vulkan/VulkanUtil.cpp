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

#include "vulkan/VulkanUtil.hpp"
#include <algorithm>

namespace Jikken
{
	namespace vkutils
	{
		std::string _vendorFromId(const uint32_t id)
		{
			switch (id)
			{
			case 0x1002: return std::string("AMD");
			case 0x1010: return std::string("ImgTec");
			case 0x10DE: return std::string("NVIDIA");
			case 0x13B5: return std::string("ARM");
			case 0x5143: return std::string("Qualcomm");
			case 0x8086: return std::string("INTEL");
			default: return std::string("unknown");
			}
		}

		std::string _deviceTypeFromId(const uint32_t id)
		{
			switch (id)
			{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER: return std::string("Other");
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return std::string("Intergrated GPU");
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return std::string("Discrete GPU");
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return std::string("Virtaul GPU");
			case VK_PHYSICAL_DEVICE_TYPE_CPU: return std::string("CPU");
			default: return std::string("unknown");
			}
		}

		void printDeviceInfo(const VkPhysicalDevice device)
		{
			if (!device)
				return;

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(device, &properties);
			uint32_t vulkMajor = VK_VERSION_MAJOR(properties.apiVersion);
			uint32_t vulkMinor = VK_VERSION_MINOR(properties.apiVersion);
			uint32_t vulkPatch = VK_VERSION_PATCH(properties.apiVersion);
			uint32_t drivMajor = VK_VERSION_MAJOR(properties.driverVersion);
			uint32_t drivMinor = VK_VERSION_MINOR(properties.driverVersion);
			uint32_t drivPatch = VK_VERSION_PATCH(properties.driverVersion);
			std::string vendor = _vendorFromId(properties.vendorID);
			std::string deviceType = _deviceTypeFromId(properties.deviceType);
			std::printf("Vendor name: %s\n", vendor.c_str());
			std::printf("Device name: %s\n", properties.deviceName);
			std::printf("Device type: %s\n", deviceType.c_str());
			std::printf("Driver version: %u.%u.%u\n", drivMajor, drivMinor, drivPatch);
			std::printf("Vulkan version: %u.%u.%u\n", vulkMajor, vulkMinor, vulkPatch);
		}

		bool checkExtension(const std::string &extensionName, const std::vector<VkExtensionProperties> &extensionList)
		{
			for (const auto &ext : extensionList)
			{
				if (extensionName == std::string(ext.extensionName))
					return true;
			}
			return false;
		}

		bool checkLayer(const std::string &layerName, const std::vector<VkLayerProperties> &layerList)
		{
			for (const auto &layer : layerList)
			{
				if (layerName == std::string(layer.layerName))
					return true;
			}
			return false;
		}

		bool checkPhysicalDevice(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface, uint32_t &graphicsQueue, uint32_t &computeQueue)
		{
			uint32_t extensionsCount = 0;
			VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, nullptr);
			if (result != VK_SUCCESS || extensionsCount == 0)
				return false;

			std::vector<VkExtensionProperties> extensionList(extensionsCount);
			result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionsCount, extensionList.data());
			if (result != VK_SUCCESS)
				return false;

			std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			//check we got required extensions
			for (const auto &ext : requiredExtensions)
			{
				if (!checkExtension(ext, extensionList))
					return false;
			}

			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;

			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

			uint32_t queueFamiliesCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
			if (queueFamiliesCount == 0)
				return false;

			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamiliesCount);

			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilyProperties.data());

			for (uint32_t i = 0; i < queueFamiliesCount; i++)
			{
				VkBool32 supportsPresent;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent);
				if (supportsPresent && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					graphicsQueue = i;

				if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					computeQueue = i;

				if (graphicsQueue != UINT32_MAX && computeQueue != UINT32_MAX)
					return true;

			}

			return false;
		}

		uint32_t getSwapChainNumImages(const VkSurfaceCapabilitiesKHR &surfaceCaps)
		{
			uint32_t count = surfaceCaps.minImageCount + 1;
			if ((surfaceCaps.maxImageCount > 0) && (count > surfaceCaps.maxImageCount))
			{
				count = surfaceCaps.maxImageCount;
			}
			return count;
		}

		VkSurfaceFormatKHR getSwapChainFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
		{
			// If the list contains only one entry with undefined format
			// it means that there are no preferred surface formats and any can be chosen
			if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
				return{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

			// Check if list contains most widely used R8 G8 B8 A8 format
			// with nonlinear color space
			for (const auto &surfaceformat : surfaceFormats)
			{
				if (surfaceformat.format == VK_FORMAT_R8G8B8A8_UNORM)
					return surfaceformat;
			}

			// Return the first format from the list
			return surfaceFormats[0];
		}


		VkSurfaceTransformFlagBitsKHR getSwapChainTransform(const VkSurfaceCapabilitiesKHR &surfaceCaps)
		{
			if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
				return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			else
				return surfaceCaps.currentTransform;
		}

		VkPresentModeKHR getSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes)
		{
			
			// Doesn't vsync or tear, more latency than tearing
			for (const auto &presentMode : presentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					return presentMode;
			}

			// FIFO present should always be available on all implementations
			for (const auto &presentMode : presentModes)
			{
				if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
					return presentMode;
			}

			std::printf("FIFO present mode is not supported by the swap chain");
			return static_cast<VkPresentModeKHR>(-1);
		}

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				return capabilities.currentExtent;
			else
			{
				VkExtent2D actualExtent;

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

				return actualExtent;
			}
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
			int32_t code, const char* layerPrefix, const char* msg, void* userData)
		{
			std::printf("Vulkan Debug: %s\n", msg);
			return VK_FALSE;
		}
	}
}