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
#include "jikken/memory.hpp"
#include "jikken/jikken.hpp"

#ifdef JIKKEN_OPENGL
#include "GL/GLGraphicsDevice.hpp"
#endif
#ifdef JIKKEN_VULKAN
#include "vulkan/VulkanGraphicsDevice.hpp"
#endif
#ifdef JIKKEN_D3D11
#include "d3d11/D3D11GraphicsDevice.hpp"
#endif

#include <SPIRV/GlslangToSpv.h>

namespace Jikken
{
	//globals
	GraphicsDevice *pGraphicsDevice = nullptr;

	bool init(const GraphicsApi api, const ContextConfig &contextConfig, const NativeWindowData &windowData)
	{
		//init glslang process
		glslang::InitializeProcess();

		if (api == GraphicsApi::eOpenGL)
		{
#ifdef JIKKEN_OPENGL
			pGraphicsDevice = new GLGraphicsDevice();
#else
			assert(false);
            return false;
#endif
		}
		else if (api == GraphicsApi::eVulkan)
		{
#ifdef JIKKEN_VULKAN
			pGraphicsDevice = new VulkanGraphicsDevice();
#else
			assert(false);
            return false;
#endif
		}
		else if (api == GraphicsApi::eDirect3D11)
		{
#ifdef JIKKEN_VULKAN
			pGraphicsDevice = new D3D11GraphicsDevice();
#else
			assert(false);
			return false;
#endif
		}
		else //not yet implemented
		{
			assert(false);
            return false;
		}

		return pGraphicsDevice->init(contextConfig, windowData);
	}

	void shutdown()
	{
		delete pGraphicsDevice;
		pGraphicsDevice = nullptr;
		glslang::FinalizeProcess();
	}

	CommandQueue* createCommandQueue()
	{
		return pGraphicsDevice->createCommandQueue();
	}

	ShaderHandle createShader(const std::vector<ShaderDetails> &shaders)
	{
		return pGraphicsDevice->createShader(shaders);
	}

	BufferHandle createBuffer(BufferType type)
	{
		return pGraphicsDevice->createBuffer(type);
	}

	LayoutHandle createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{
		return pGraphicsDevice->createVertexInputLayout(attributes);
	}

	VertexArrayHandle createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer)
	{
		return pGraphicsDevice->createVAO(layout, vertexBuffer, indexBuffer);
	}

	//deletion
	void deleteCommandQueue(CommandQueue *cmdQueue)
	{
		pGraphicsDevice->deleteCommandQueue(cmdQueue);
	}

	void deleteVertexInputLayout(LayoutHandle handle)
	{
		pGraphicsDevice->deleteVertexInputLayout(handle);
	}

	void deleteVAO(VertexArrayHandle handle)
	{
		pGraphicsDevice->deleteVAO(handle);
	}

	void deleteBuffer(BufferHandle handle)
	{
		pGraphicsDevice->deleteBuffer(handle);
	}

	void deleteShader(ShaderHandle handle)
	{
		pGraphicsDevice->deleteShader(handle);
	}

	//queue submission
	void submitCommandQueue(CommandQueue *queue)
	{
		pGraphicsDevice->submitCommandQueue(queue);
	}

	//execute 
	void execute(bool presentToScreen)
	{
		pGraphicsDevice->execute(presentToScreen);
	}

	CommandQueue* getImmediateExecuteQueue()
	{
		return pGraphicsDevice->getImmediateExecuteQueue();
	}

	void executeImmediateQueue()
	{
		pGraphicsDevice->executeImmediateQueue();
	}

	// todo - this shouldn't be here - make command for this
	void bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index)
	{
		pGraphicsDevice->bindConstantBuffer(shader, cBuffer, name, index);
	}

	void resize(const int32_t width, const int32_t height)
	{
	}

}
