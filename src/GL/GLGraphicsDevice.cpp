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
			_setShaderCmd(static_cast<SetShaderCommand*>(cmd));
			break;
		case CommandType::eUpdateBuffer:
			_updateBufferCmd(static_cast<UpdateBufferCommand*>(cmd));
			break;
		case CommandType::eDraw:
			_drawCmd(static_cast<DrawCommand*>(cmd));
			break;
		case CommandType::eDrawInstance:
			_drawInstanceCmd(static_cast<DrawInstanceCommand*>(cmd));
			break;
		case CommandType::eClearBuffer:
			_clearBufferCmd(static_cast<ClearBufferCommand*>(cmd));
			break;
		case CommandType::eBindVAO:
			_bindVAOCmd(static_cast<BindVAOCommand*>(cmd));
			break;
		case CommandType::eViewport:
			_viewportCmd(static_cast<ViewportCommand*>(cmd));
			break;
		case CommandType::eBlendState:
			_blendStateCmd(static_cast<BlendStateCommand*>(cmd));
			break;
		case CommandType::eDepthStencilState:
			_depthStencilStateCmd(static_cast<DepthStencilStateCommand*>(cmd));
			break;
		case CommandType::eCullState:
			_cullStateCmd(static_cast<CullStateCommand*>(cmd));
			break;
		default:
			printf("Invalid command to process in GLGraphicsDevice: %i", cmd->commandType);
			break;
		}
	}

	void GLGraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{

	}

	void GLGraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{

	}

	void GLGraphicsDevice::_drawCmd(DrawCommand *cmd)
	{

	}

	void GLGraphicsDevice::_drawInstanceCmd(DrawInstanceCommand *cmd)
	{

	}

	void GLGraphicsDevice::_clearBufferCmd(ClearBufferCommand *cmd)
	{

	}

	void GLGraphicsDevice::_bindVAOCmd(BindVAOCommand *cmd)
	{

	}

	void GLGraphicsDevice::_viewportCmd(ViewportCommand *cmd)
	{

	}

	void GLGraphicsDevice::_blendStateCmd(BlendStateCommand *cmd)
	{

	}

	void GLGraphicsDevice::_depthStencilStateCmd(DepthStencilStateCommand *cmd)
	{

	}

	void GLGraphicsDevice::_cullStateCmd(CullStateCommand *cmd)
	{

	}
}