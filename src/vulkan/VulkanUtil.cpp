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

		uint32_t findMemoryType(const VkPhysicalDevice physicalDevice, const uint32_t typeFilter, const VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			return UINT32_MAX;
		}

		void beingSingleCommand(const VkCommandBuffer cmdBuffer)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(cmdBuffer, &beginInfo);
		}

		void endSingleCommand(const VkCommandBuffer cmdBuffer, const VkQueue queue)
		{
			vkEndCommandBuffer(cmdBuffer);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmdBuffer;

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);
		}

		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			// Create an image barrier object
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source 
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from and writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			}

			// Put barrier inside setup command buffer
			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		// Fixed sub resource on first mip level and layer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;
			setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange);
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
			int32_t code, const char* layerPrefix, const char* msg, void* userData)
		{
			std::printf("Vulkan Debug: %s\n", msg);
			return VK_FALSE;
		}
	}
}