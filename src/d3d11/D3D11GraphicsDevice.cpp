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
#include "d3d11/D3D11GraphicsDevice.hpp"
#include "d3d11/D3D11Util.hpp"
#include <array>
#include "shaderUtils.hpp"
//just temp for shader loading
#include <fstream>
#include <sstream>
#include <d3d11sdklayers.h>

//libs needed for d3d11
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Jikken
{

	D3D11GraphicsDevice::D3D11GraphicsDevice() :
		mBufferHandle(0),
		mVertexArrayHandle(0),
		mShaderHandle(0),
		mLayoutHandle(0),
		mDevice(nullptr),
		mDeviceContext(nullptr),
		mDebugEnabled(false)
	{
	}

	D3D11GraphicsDevice::~D3D11GraphicsDevice()
	{
		//release device context
		d3d11utils::SafeRelease(&mDeviceContext);
		
		//report live objects and than release debug interface
		if (mDebugEnabled)
		{
			ID3D11Debug *pDebug = nullptr;
			mDevice->QueryInterface(IID_PPV_ARGS(&pDebug));
			pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			d3d11utils::SafeRelease(&pDebug);
		}

		//release device
		d3d11utils::SafeRelease(&mDevice);
	}

	bool D3D11GraphicsDevice::init(const ContextConfig &contextConfig, const NativeWindowData &windowData)
	{

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		if (contextConfig.debugEnabled)
		{
			createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			mDebugEnabled = true;
		}
		
		//create ID3D11Device & ID3D11DeviceContext
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mDeviceContext);

		if (FAILED(result))
		{
			// if debug enabled let's try again without the debug device
			if (mDebugEnabled)
			{
				createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
				result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mDeviceContext);
				if (FAILED(result))
				{
					std::printf("Failed to create D3D11 device\n");
					return false;
				}

				std::printf("D3D11 debugging disabled\n");
				mDebugEnabled = false;
			}
			else
			{
				std::printf("Failed to create D3D11 device\n");
				return false;
			}
		}


		return true;
	}


	ShaderHandle D3D11GraphicsDevice::createShader(const std::vector<ShaderDetails> &shaders)
	{
		//check if this will put us over the maximum number of shader handles
		if ((mShaderHandle + 1) == InvalidHandle)
		{
			std::printf("Too many shader handles");
			return InvalidHandle;
		}

	
		return InvalidHandle;
	}

	BufferHandle D3D11GraphicsDevice::createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, void *data)
	{
		return InvalidHandle;
	}

	LayoutHandle D3D11GraphicsDevice::createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{
		return InvalidHandle;
	}

	VertexArrayHandle D3D11GraphicsDevice::createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer)
	{
		return InvalidHandle;
	}

	void D3D11GraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index)
	{
	}

	void D3D11GraphicsDevice::deleteVertexInputLayout(LayoutHandle handle)
	{
	}

	void D3D11GraphicsDevice::deleteVAO(VertexArrayHandle handle)
	{
	}

	void D3D11GraphicsDevice::deleteBuffer(BufferHandle handle)
	{
	}

	void D3D11GraphicsDevice::deleteShader(ShaderHandle handle)
	{
	}

	void D3D11GraphicsDevice::presentFrame()
	{
	}

	//Commands
	void D3D11GraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_beginFrameCmd(BeginFrameCommand *cmd)
	{		
	}

	void D3D11GraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_reallocBufferCmd(ReallocBufferCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_drawCmd(DrawCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_drawInstanceCmd(DrawInstanceCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_clearBufferCmd(ClearBufferCommand *cmd)
	{
	}

	//we have to fake this as VAO is an opengl only concept
	void D3D11GraphicsDevice::_bindVAOCmd(BindVAOCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_viewportCmd(ViewportCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_blendStateCmd(BlendStateCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_depthStencilStateCmd(DepthStencilStateCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_cullStateCmd(CullStateCommand *cmd)
	{
	}
}