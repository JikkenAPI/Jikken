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
		mDebugEnabled(false),
		mPresentInterval(0)
	{
	}

	D3D11GraphicsDevice::~D3D11GraphicsDevice()
	{
		//check for any buffers not deleted by bad users
		for (auto &buffer : mBuffers)
		{
			std::printf("Releasing a buffer resource that was not freed!");
			d3d11utils::SafeRelease(&buffer.second.buffer);			
		}

		//cleanup swapchain
		d3d11utils::SafeRelease(&mSwapChain.depthStencilView);
		d3d11utils::SafeRelease(&mSwapChain.depthStencil);
		d3d11utils::SafeRelease(&mSwapChain.backBufferView);
		d3d11utils::SafeRelease(&mSwapChain.backBuffer);
		d3d11utils::SafeRelease(&mSwapChain.swapChain);

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

		//present interval at creation time
		mPresentInterval = contextConfig.vsyncEnabled ? 1 : 0;

		//create swap chain
		IDXGIFactory1* pDXGIFactory;
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDXGIFactory));
		if (FAILED(result))
		{
			std::printf("Failed to create a IDXGIFactory\n");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		DXGI_FORMAT backBufferFmt = contextConfig.srgbEnabled ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
		swapDesc.BufferCount = contextConfig.vsyncEnabled ? 2 : 1;// fullscreen will be 3:2
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.BufferDesc.Width = windowData.fbWidth;
		swapDesc.BufferDesc.Height = windowData.fbHeight;
		swapDesc.BufferDesc.Format = backBufferFmt;
		swapDesc.OutputWindow = static_cast<HWND>(windowData.handle);
		swapDesc.Windowed = TRUE; //todo fullscreen support
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = contextConfig.msaaLevel;

		result = pDXGIFactory->CreateSwapChain(mDevice, &swapDesc, &mSwapChain.swapChain);
		d3d11utils::SafeRelease(&pDXGIFactory);
		if (FAILED(result))
		{
			std::printf("Failed to create swap chain");
			return false;
		}

		//backbuffer texture
		mSwapChain.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mSwapChain.backBuffer);

		//backbuffer view
		D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc = {};
		rtViewDesc.Format = backBufferFmt;
		rtViewDesc.Texture2D.MipSlice = 0;
		rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		result = mDevice->CreateRenderTargetView(mSwapChain.backBuffer, &rtViewDesc, &mSwapChain.backBufferView);
		if (FAILED(result))
		{
			std::printf("Failed to create backbuffer view");
			return false;
		}

		//depth stencil
		if (contextConfig.depthBits > 0)
		{
			//create depth/stencil texture
			DXGI_FORMAT depthStencilFmt = DXGI_FORMAT_D24_UNORM_S8_UINT;//todo pick format based on depthBits & stencilBits
			D3D11_TEXTURE2D_DESC depthStencilDesc;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthStencilDesc.CPUAccessFlags = 0;
			depthStencilDesc.Format = depthStencilFmt;
			depthStencilDesc.Width = windowData.fbWidth;
			depthStencilDesc.Height = windowData.fbHeight;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
			depthStencilDesc.MiscFlags = 0;
			result = mDevice->CreateTexture2D(&depthStencilDesc, nullptr, &mSwapChain.depthStencil);
			if (FAILED(result))
			{
				std::printf("Failed to create dpeth/stencil texture");
				return false;
			}

			//create depth/stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			depthStencilViewDesc.Format = depthStencilFmt;
			depthStencilViewDesc.Flags = 0;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;
			result = mDevice->CreateDepthStencilView(mSwapChain.depthStencil, &depthStencilViewDesc, &mSwapChain.depthStencilView);
			if (FAILED(result))
			{
				std::printf("Failed to create dpeth/stencil view");
				return false;
			}
		}

		//bind default backbuffer
		mDeviceContext->OMSetRenderTargets(1, &mSwapChain.backBufferView, mSwapChain.depthStencilView);

		//set the viewport
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(windowData.fbWidth);
		viewport.Height = static_cast<float>(windowData.fbHeight);
		mDeviceContext->RSSetViewports(1, &viewport);

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

	BufferHandle D3D11GraphicsDevice::createBuffer(BufferType type)
	{
		BufferHandle handle = mBufferHandle++;
		mBuffers[handle] = { type, nullptr };
		return handle;
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
		d3d11utils::SafeRelease(&mBuffers[handle].buffer);
		mBuffers.erase(handle);
	}

	void D3D11GraphicsDevice::deleteShader(ShaderHandle handle)
	{
	}

	void D3D11GraphicsDevice::_presentCmd()
	{
		HRESULT result = mSwapChain.swapChain->Present(mPresentInterval, 0);
		if (FAILED(result))
		{
			std::printf("Failed to present frame");
		}
	}

	//Commands
	void D3D11GraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_beginFrameCmd(BeginFrameCommand *cmd)
	{
		uint32_t depthstencilFlag = 0;
		if (cmd->clearFlag & ClearBufferFlags::eColor)
			mDeviceContext->ClearRenderTargetView(mSwapChain.backBufferView, cmd->clearColor);

		if (cmd->clearFlag & ClearBufferFlags::eDepth)
			depthstencilFlag |= D3D11_CLEAR_DEPTH;

		if (cmd->clearFlag & ClearBufferFlags::eStencil)
			depthstencilFlag |= D3D11_CLEAR_STENCIL;

		if (depthstencilFlag)
			mDeviceContext->ClearDepthStencilView(mSwapChain.depthStencilView, depthstencilFlag, cmd->depth, static_cast<UINT8>(cmd->stencil));
	}

	void D3D11GraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{
	}

	void D3D11GraphicsDevice::_allocBufferCmd(AllocBufferCommand *cmd)
	{
		uint32_t bindFlags = 0;
		uint32_t cpuAccessFlags = 0;
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		ID3D11Buffer *pBuffer = mBuffers[cmd->buffer].buffer;

		switch (cmd->hint)
		{
		case BufferUsageHint::eDynamic:
			usage = D3D11_USAGE_DYNAMIC;
			cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
			break;
		case BufferUsageHint::eImmutable:
			usage = D3D11_USAGE_IMMUTABLE;
			cpuAccessFlags = 0;
			break;
		case BufferUsageHint::eStatic:
			usage = D3D11_USAGE_DEFAULT;
			cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
			break;
		}

		switch (mBuffers[cmd->buffer].type)
		{
		case BufferType::eVertexBuffer:
			bindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferType::eIndexBuffer:
			bindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferType::eConstantBuffer:
			usage = D3D11_USAGE_DYNAMIC; //over-ride usage
			bindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cpuAccessFlags = D3D11_CPU_ACCESS_WRITE; //over-ride access flags
			break;
		default:
			std::printf("Invalid buffer type");
			assert(false);
			return;
		}

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.CPUAccessFlags = cpuAccessFlags;
		bufferDesc.BindFlags = bindFlags;
		bufferDesc.ByteWidth = static_cast<UINT>(cmd->dataSize);
		bufferDesc.Usage = usage;

		HRESULT result;
		if (cmd->data)
		{
			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = cmd->data;
			result = mDevice->CreateBuffer(&bufferDesc, &initData, &pBuffer);
		}
		else
		{
			//double check we are not immutable type
			if (cmd->hint == BufferUsageHint::eImmutable)
			{
				std::printf("Data must be supplied on creation when using eImmutable");
				return;
			}
			result = mDevice->CreateBuffer(&bufferDesc, nullptr, &pBuffer);
		}

		if (FAILED(result))
		{
			std::printf("Failed to create buffer");
			assert(false);
		}
	
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