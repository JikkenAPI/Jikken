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

#ifndef _JIKKEN_COMMANDQUEUE_HPP_
#define _JIKKEN_COMMANDQUEUE_HPP_

#include "jikken/commands.hpp"
#include "jikken/memory.hpp"
#include <cstring>

namespace Jikken
{
	class CommandQueue
	{
		friend class GraphicsDevice;
	private:

		struct Packet
		{
			void *mem;
			Packet *next;

			Packet() :mem(nullptr), next(nullptr) {}
		};

		CommandQueue() :
			mCmdMemory(4096, 4),
			mBufferMemory(MemoryPool::MEGABYTE * 4, 1),
			mFirstPacket(nullptr),
			mLastPacket(nullptr),
			mCurrentReadPacket(nullptr)
		{
		}

		~CommandQueue()
		{
		}

	private:

		inline void writeCmd(const void* data, size_t size)
		{
			Packet *packet = mCmdMemory.malloc<Packet>();
			packet->mem = mCmdMemory.malloc(size);
			memcpy(packet->mem, data, size);

			if (mLastPacket != nullptr)
				mLastPacket->next = packet;
			else
				mFirstPacket = packet;

			mLastPacket = packet;
		}

		template<typename T>
		inline void writeCmd(const T& _in)
		{
			writeCmd(reinterpret_cast<const void*>(&_in), sizeof(T));
		}

		inline void writeData(const void* data, size_t size)
		{
			void* mem = mBufferMemory.malloc(size);
			memcpy(mem, data, size);
			//write pointer address to the command buffer for later retrieval
			uintptr_t addr = reinterpret_cast<uintptr_t>(mem);
			writeCmd(addr);
		}

		template<typename T>
		inline void writeData(const T& _in)
		{
			writeData(reinterpret_cast<const void*>(&_in), sizeof(T));
		}

		inline void reset()
		{
			mBufferMemory.free();
			mCmdMemory.free();
			mFirstPacket = nullptr;
			mLastPacket = nullptr;
			mCurrentReadPacket = nullptr;
		}

		inline void finish()
		{
			uint8_t finish = eFinishQueue;
			writeCmd(finish);
			mCurrentReadPacket = mFirstPacket;
		}

		inline void readCmd(void* data, size_t size)
		{
			assert(mCurrentReadPacket);
			assert(mCurrentReadPacket->mem);

			memcpy(data, mCurrentReadPacket->mem, size);
			mCurrentReadPacket = mCurrentReadPacket->next;
		}

		template<typename T>
		inline void readCmd(T& _in)
		{
			readCmd(reinterpret_cast<void*>(&_in), sizeof(T));
		}

	public:

		//add/record commands to the queue
		inline void addSetShaderCommand(const SetShaderCommand *cmd)
		{
			writeCmd(eSetShader);
			writeCmd(cmd,sizeof(SetShaderCommand));
		}

		inline void addBeginFrameCommand(const BeginFrameCommand *cmd)
		{
			writeCmd(eBeginFrame);
			writeCmd(cmd,sizeof(BeginFrameCommand));
		}

		inline void addUpdateBufferCommand(const UpdateBufferCommand *cmd)
		{
			writeCmd(eUpdateBuffer);
			writeCmd(cmd->buffer);
			writeCmd(cmd->dataSize);
			writeCmd(cmd->offset);
			//write data
			writeData(cmd->data, cmd->dataSize);
		}

		inline void addAllocBufferCommand(const AllocBufferCommand *cmd)
		{
			writeCmd(eAllocBuffer);
			writeCmd(cmd->buffer);
			writeCmd(cmd->dataSize);
			writeCmd(cmd->hint);
			//write data if any - it's ok to alloc a buffer with nullptr
			if (cmd->data)
			{
				writeCmd(static_cast<uint8_t>(1));//has data flag
				writeData(cmd->data, cmd->dataSize);
			}
			else
			{
				writeCmd(static_cast<uint8_t>(0));//no data flag
			}
		}

		inline void addClearCommand(const ClearBufferCommand *cmd)
		{
			writeCmd(eClearBuffer);
			writeCmd(cmd, sizeof(ClearBufferCommand));
		}

		inline void addDepthStencilStateCommand(const DepthStencilStateCommand *cmd)
		{
			writeCmd(eDepthStencilState);
			writeCmd(cmd, sizeof(DepthStencilStateCommand));
		}

		inline void addBlendStateCommand(const BlendStateCommand *cmd)
		{
			writeCmd(eBlendState);
			writeCmd(cmd, sizeof(BlendStateCommand));
		}

		inline void addCullStateCommand(const CullStateCommand *cmd)
		{
			writeCmd(eCullState);
			writeCmd(cmd, sizeof(CullStateCommand));
		}

		inline void addBindVAOCommand(const BindVAOCommand *cmd)
		{
			writeCmd(eBindVAO);
			writeCmd(cmd, sizeof(BindVAOCommand));
		}

		inline void addDrawCommand(const DrawCommand *cmd)
		{
			writeCmd(eDraw);
			writeCmd(cmd, sizeof(DrawCommand));
		}


	private:

		MemoryPool mBufferMemory;
		MemoryPool mCmdMemory;

		Packet *mFirstPacket;
		Packet *mLastPacket;
		Packet *mCurrentReadPacket;
	};
}

#endif
