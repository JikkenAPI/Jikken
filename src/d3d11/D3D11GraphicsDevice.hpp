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

#ifndef _JIKKEN_D3D11_D3D11GRAPHICSDEVICE_HPP_
#define _JIKKEN_D3D11_D3D11GRAPHICSDEVICE_HPP_

#include <unordered_map>
#include "graphicsDevice.hpp"
#include <d3d11.h>


namespace Jikken
{
	class D3D11GraphicsDevice : public GraphicsDevice
	{
	public:
		D3D11GraphicsDevice();
		virtual ~D3D11GraphicsDevice();

		virtual ShaderHandle createShader(const std::vector<ShaderDetails> &shaders) override;

		virtual BufferHandle createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, void *data) override;

		virtual LayoutHandle createVertexInputLayout(const std::vector<VertexInputLayout> &attributes) override;

		virtual VertexArrayHandle createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = InvalidHandle) override;

		virtual void bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index) override;

		virtual void deleteVertexInputLayout(LayoutHandle handle) override;

		virtual void deleteVAO(VertexArrayHandle handle) override;

		virtual void deleteBuffer(BufferHandle handle) override;

		virtual void deleteShader(ShaderHandle handle) override;

		virtual bool init(const ContextConfig &contextConfig, const NativeWindowData &windowData) override;

		virtual void presentFrame() override;

	protected:

		virtual void _setShaderCmd(SetShaderCommand *cmd) override;
		virtual void _beginFrameCmd(BeginFrameCommand *cmd) override;
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

	
		BufferHandle mBufferHandle;
		VertexArrayHandle mVertexArrayHandle;
		ShaderHandle mShaderHandle;
		LayoutHandle mLayoutHandle;

		ID3D11Device* mDevice;
		ID3D11DeviceContext* mDeviceContext;
		D3D_FEATURE_LEVEL mFeatureLevel;
		bool mDebugEnabled;

	};
}

#endif