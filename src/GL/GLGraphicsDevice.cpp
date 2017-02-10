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

#include "GLGraphicsDevice.hpp"

namespace Jikken
{
	GLGraphicsDevice::GLGraphicsDevice()
	{
	
	}

	GLGraphicsDevice::~GLGraphicsDevice()
	{

	}

	ShaderHandle GLGraphicsDevice::createShader(const std::vector<ShaderDetails> &shaders)
	{

	}

	BufferHandle GLGraphicsDevice::createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, float *data)
	{

	}

	LayoutHandle GLGraphicsDevice::createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{

	}

	VertexArrayHandle GLGraphicsDevice::createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = 0)
	{

	}

	void GLGraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, int32_t index)
	{

	}

	void GLGraphicsDevice::deleteVertexInputLayout(LayoutHandle handle)
	{

	}

	void GLGraphicsDevice::deleteVAO(VertexArrayHandle handle)
	{

	}

	void GLGraphicsDevice::deleteBuffer(BufferHandle handle)
	{

	}

	void GLGraphicsDevice::deleteShader(ShaderHandle handle)
	{

	}

	void GLGraphicsDevice::submitCommandQueue()
	{
		for (CommandQueue *cmdQueue : mCommandQueuePool)
		{
			// Process all commands and then clear the queue so it can be filled again.
			for (ICommand *cmd = cmdQueue->beginList(); cmd != nullptr; cmd = cmd->next)
				_processCmd(cmd);
			cmdQueue->resetQueue();
		}
	}

	void GLGraphicsDevice::_processCmd(ICommand *cmd)
	{
		switch (cmd->commandType)
		{
		case CommandType::eSetShader:
			break;
		case CommandType::eUpdateConstant:
			break;
		case CommandType::eDraw:
			break;
		case CommandType::eDrawInstance:
			break;
		case CommandType::eClearBuffer:
			break;
		case CommandType::eBindVAO:
			break;
		case CommandType::eViewport:
			break;
		case CommandType::eBlendState:
			break;
		case CommandType::eDepthStencilState:
			break;
		case CommandType::eCullState:
			break;
		}
	}
}