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

#include "jikken/graphicsDevice.hpp"
#include <algorithm>

namespace Jikken
{
	GraphicsDevice::GraphicsDevice()
	{
	}

	GraphicsDevice::~GraphicsDevice()
	{
		// Cleanup all command queues.
		for (CommandQueue *queue : mCommandQueuePool)
		{
			delete queue;
			queue = nullptr;
		}
	}

	CommandQueue* GraphicsDevice::createCommandQueue()
	{
		CommandQueue *queue = new CommandQueue;
		mCommandQueuePool.push_back(queue);
		return queue;
	}

	void GraphicsDevice::deleteCommandQueue(CommandQueue *cmdQueue)
	{
		auto pos = std::find(mCommandQueuePool.begin(), mCommandQueuePool.end(), cmdQueue);
		mCommandQueuePool.erase(pos);
		delete cmdQueue;
		cmdQueue = nullptr;
	}

	void GraphicsDevice::submitCommandQueue(CommandQueue *queue)
	{
		//mark queue as finished (i.e write eFinishQueue)
		queue->finish();

	    //decode all commands and execute them
		bool queueEnd = false;
		while (!queueEnd)
		{
			uint8_t cmdType;
			queue->readCmd(cmdType);
			switch (cmdType)
			{
			case eSetShader:
			{
				SetShaderCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_setShaderCmd(&cmd);
				break;
			}

			case eBeginFrame:
			{
				BeginFrameCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_beginFrameCmd(&cmd);
				break;
			}

			case eDepthStencilState:
			{
				DepthStencilStateCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_depthStencilStateCmd(&cmd);
				break;
			}

			case eDraw:
			{
				DrawCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_drawCmd(&cmd);
				break;
			}

			case eUpdateBuffer:
			{
				UpdateBufferCommand cmd;
				queue->readCmd(cmd.buffer);
				queue->readCmd(cmd.dataSize);
				queue->readCmd(cmd.offset);
				//read data pointer address
				uintptr_t addr;
				queue->readCmd(addr);
				cmd.data = reinterpret_cast<void*>(addr);
				//execute cmd
				_updateBufferCmd(&cmd);
				break;
			}

			case eReallocBuffer:
			{
				ReallocBufferCommand cmd;
				queue->readCmd(cmd.buffer);
				queue->readCmd(cmd.count);
				queue->readCmd(cmd.hint);
				queue->readCmd(cmd.stride);
				//read data pointer address
				uintptr_t addr;
				queue->readCmd(addr);
				cmd.data = reinterpret_cast<void*>(addr);
				//execute cmd
				_reallocBufferCmd(&cmd);
				break;
			}

			case eBindVAO:
			{
				BindVAOCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_bindVAOCmd(&cmd);
				break;
			}

			case eCullState:
			{
				CullStateCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_cullStateCmd(&cmd);
				break;
			}

			case eClearBuffer:
			{
				ClearBufferCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_clearBufferCmd(&cmd);
				break;
			}

			case eBlendState:
			{
				BlendStateCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_blendStateCmd(&cmd);
				break;
			}

			case eViewport:
			{
				ViewportCommand cmd;
				queue->readCmd(cmd);
				//execute cmd
				_viewportCmd(&cmd);
				break;
			}

			case eFinishQueue:
				queueEnd = true;
				break;

			default:
				assert(false);
				break;
			}
		}

		//reset queue so it can be used again
		queue->reset();
	}
}
