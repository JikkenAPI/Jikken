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

#ifndef _JIKKEN_VULKAN_VULKANUTIL_HPP_
#define _JIKKEN_VULKAN_VULKANUTIL_HPP_

#include "jikken/enums.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Jikken
{
	namespace vkutils
	{
		bool checkExtension(const std::string &extensionName, const std::vector<VkExtensionProperties> &extensionList);
		bool checkLayer(const std::string &layerName, const std::vector<VkLayerProperties> &layerList);
		bool checkPhysicalDevice(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface, uint32_t &graphicsQueue, uint32_t &computeQueue);
		uint32_t findMemoryType(const VkPhysicalDevice physicalDevice, const uint32_t typeFilter, const VkMemoryPropertyFlags properties);
		void printDeviceInfo(const VkPhysicalDevice device);

		//execute single commands
		void beingSingleCommand(const VkCommandBuffer cmdBuffer);
		void endSingleCommand(const VkCommandBuffer cmdBuffer, const VkQueue queue);

		// based from https://github.com/SaschaWillems/Vulkan
		// Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		// Uses a fixed sub resource layout with first mip level and layer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		uint32_t getSwapChainNumImages(const VkSurfaceCapabilitiesKHR &surfaceCaps);
		//todo add requested format
		VkSurfaceFormatKHR getSwapChainFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats);
		VkSurfaceTransformFlagBitsKHR getSwapChainTransform(const VkSurfaceCapabilitiesKHR &surfaceCaps);
		//todo should really let user have a say in this
		VkPresentModeKHR getSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);
		//choose swapchain extent
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		//VkShaderStageFlagBits - todo use static lookup tables
		VkShaderStageFlagBits getShaderStageFlag(const ShaderStage stage);

		//debug callback
		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
			int32_t code, const char* layerPrefix, const char* msg, void* userData);
	}
}

#endif