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

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Jikken
{
	namespace vkutils
	{
		bool checkExtension(const std::string &extensionName, const std::vector<VkExtensionProperties> &extensionList);
		bool checkLayer(const std::string &layerName, const std::vector<VkLayerProperties> &layerList);

		//debug callback
		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
			int32_t code, const char* layerPrefix, const char* msg, void* userData);
	}
}

#endif