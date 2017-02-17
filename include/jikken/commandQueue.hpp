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

#define MAX_COMMAND_BUFFER_SIZE (64<<10)

//TODO mCmdMemory & mBufferMemory pools
namespace Jikken
{
	class CommandQueue
	{
		friend class GraphicsDevice;
	private:
		CommandQueue() :
			mPos(0),
			mSize(MAX_COMMAND_BUFFER_SIZE),
			mCmdMemory(4096, 4),
			mBufferMemory(MemoryPool::MEGABYTE * 4, 1)
		{
			finish();
		}

		~CommandQueue()
		{
		}


	private:

		inline void write(const void* data, size_t size)
		{
			memcpy(&mBuffer[mPos], data, size);
			mPos += size;
		}

		template<typename T>
		inline void write(const T& _in)
		{
			write(reinterpret_cast<const uint8_t*>(&_in), sizeof(T));
		}

	public:

		inline void reset()
		{
			mPos = 0;
		}

		inline void finish()
		{
			uint8_t finish = eFinishQueue;
			write(finish);
			mSize = mPos;
			mPos = 0;
		}

		inline void read(void* data, size_t size)
		{
			memcpy(data, &mBuffer[mPos], size);
			mPos += size;
		}

		template<typename T>
		inline void read(T& _in)
		{
			read(reinterpret_cast<uint8_t*>(&_in), sizeof(T));
		}

		void skip(size_t size)
		{
			mPos += size;
		}

		//add/record commands to the queue
		inline void addSetShaderCommand(const SetShaderCommand *cmd)
		{
			write(eSetShader);
			write(cmd,sizeof(SetShaderCommand));
		}

		inline void addUpdateBufferCommand(const UpdateBufferCommand *cmd)
		{
			write(eUpdateBuffer);
			write(cmd->buffer);
			write(cmd->dataSize);
			write(cmd->offset);
			//write data pointer address
			uintptr_t addr = reinterpret_cast<uintptr_t>(&mBuffer[mPos + sizeof(uintptr_t)]);
			write(addr);
			//write data
			write(cmd->data, cmd->dataSize);
		}

		inline void addReallocBufferCommand(const ReallocBufferCommand *cmd)
		{
			write(eReallocBuffer);
			write(cmd->buffer);
			write(cmd->count);
			write(cmd->hint);
			write(cmd->stride);
			//write data pointer address
			uintptr_t addr = reinterpret_cast<uintptr_t>(&mBuffer[mPos + sizeof(uintptr_t)]);
			write(addr);
			//write data
			write(cmd->data, cmd->stride * cmd->count);
		}

		inline void addClearCommand(const ClearBufferCommand *cmd)
		{
			write(eClearBuffer);
			write(cmd, sizeof(ClearBufferCommand));
		}

		inline void addDepthStencilStateCommand(const DepthStencilStateCommand *cmd)
		{
			write(eDepthStencilState);
			write(cmd, sizeof(DepthStencilStateCommand));
		}

		inline void addBlendStateCommand(const BlendStateCommand *cmd)
		{
			write(eBlendState);
			write(cmd, sizeof(BlendStateCommand));
		}

		inline void addCullStateCommand(const CullStateCommand *cmd)
		{
			write(eCullState);
			write(cmd, sizeof(CullStateCommand));
		}

		inline void addBindVAOCommand(const BindVAOCommand *cmd)
		{
			write(eBindVAO);
			write(cmd, sizeof(BindVAOCommand));
		}

		inline void addDrawCommand(const DrawCommand *cmd)
		{
			write(eDraw);
			write(cmd, sizeof(DrawCommand));
		}


	private:
		uintptr_t mPos;
		size_t mSize;
		uint8_t mBuffer[MAX_COMMAND_BUFFER_SIZE];
		//TODO mCmdMemory & mBufferMemory pools to replace above mBuffer
		MemoryPool mBufferMemory;
		MemoryPool mCmdMemory;
	};
}

#endif
