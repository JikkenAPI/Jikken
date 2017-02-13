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
		mPhysicalDevice(VK_NULL_HANDLE),
		mDevice(VK_NULL_HANDLE),
		mDebugCallback(VK_NULL_HANDLE),
		mAllocCallback(nullptr)
	{
	}

	VulkanGraphicsDevice::~VulkanGraphicsDevice()
	{
	}

	bool VulkanGraphicsDevice::init(void *glfwWinHandle)
	{
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