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

#ifndef _JIKKEN_VULKAN_VULKANSTRUCTS_HPP_
#define _JIKKEN_VULKAN_VULKANSTRUCTS_HPP_

#include <vulkan/vulkan.h>

namespace Jikken
{
	struct ImageParams
	{
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VkDeviceMemory memory;

		ImageParams() :
			image(VK_NULL_HANDLE),
			view(VK_NULL_HANDLE),
			sampler(VK_NULL_HANDLE),
			memory(VK_NULL_HANDLE)
		{}
	};

	struct SwapChainParams
	{
		VkSwapchainKHR swapChain;
		VkFormat format;
		VkExtent2D extent;
		std::vector<ImageParams> images;
		uint32_t currentImage;

		SwapChainParams() :
			swapChain(VK_NULL_HANDLE),
			format(VK_FORMAT_UNDEFINED),
			extent(),
			images(),
			currentImage(0)
		{}
	};

	struct ViewportParams
	{
		VkViewport viewport;
		VkRect2D scissor;
		ViewportParams() :
			viewport(),
			scissor()
		{}
	};
}

#endif
