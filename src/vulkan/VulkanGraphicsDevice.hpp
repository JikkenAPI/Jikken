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

#ifndef _JIKKEN_VULKAN_VULKANGRAPHICSDEVICE_HPP_
#define _JIKKEN_VULKAN_VULKANGRAPHICSDEVICE_HPP_

#include <unordered_map>
#include <vulkan/vulkan.h>
#include "jikken/graphicsDevice.hpp"
#include "vulkan/VulkanStructs.hpp"

namespace Jikken
{
	class VulkanGraphicsDevice : public GraphicsDevice
	{
	public:

		VulkanGraphicsDevice();
		virtual ~VulkanGraphicsDevice();

		virtual ShaderHandle createShader(const std::vector<ShaderDetails> &shaders) override;

		virtual BufferHandle createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, float *data) override;

		virtual LayoutHandle createVertexInputLayout(const std::vector<VertexInputLayout> &attributes) override;

		virtual VertexArrayHandle createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = 0) override;

		virtual void bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index) override;

		virtual void deleteVertexInputLayout(LayoutHandle handle) override;

		virtual void deleteVAO(VertexArrayHandle handle) override;

		virtual void deleteBuffer(BufferHandle handle) override;

		virtual void deleteShader(ShaderHandle handle) override;

		virtual bool init(void *glfwWinHandle) override;

		virtual void present() override;

	protected:
		virtual void _setShaderCmd(SetShaderCommand *cmd) override;
		virtual void _updateBufferCmd(UpdateBufferCommand *cmd) override;
		virtual void _reallocBufferCmd(ReallocBufferCommand *cmd) override;
		virtual void _drawCmd(DrawCommand *cmd) override;
		virtual void _drawInstanceCmd(DrawInstanceCommand *cmd) override;
		virtual void _clearBufferCmd(ClearBufferCommand *cmd) override;
		virtual void _bindVAOCmd(BindVAOCommand *cmd) override;
		virtual void _viewportCmd(ViewportCommand *cmd) override;
		virtual void _blendStateCmd(BlendStateCommand *cmd) override;
		virtual void _depthStencilStateCmd(DepthStencilStateCommand *cmd) override;
		virtual void _cullStateCmd(CullStateCommand *cmd) override;

	private:

		//private functions
		bool _createSwapchain();

		//private variables
		VkInstance mInstance; //vulkan app instance
		VkSurfaceKHR mSurface; //window surface
		VkPhysicalDevice mPhysicalDevice; //physical device
		VkDevice mDevice; // logical device
		VkQueue mGraphicsQueue; //graphics queue
		VkQueue mComputeQueue; //compute queue
		uint32_t mGraphicsQueueIndex; //graphics queue index
		uint32_t mComputeQueueIndex; // compute queue index
		VkRenderPass mRenderPass; //render pass
		VkDebugReportCallbackEXT mDebugCallback; //debug callback
		VkAllocationCallbacks *mAllocCallback; //allocation callback
		SwapChainParams mSwapChainParams; //swap chain paramaters
		ViewportParams mViewPortParams; //viewport paramaters
	};
}

#endif