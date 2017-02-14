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

namespace Jikken
{
	class CommandQueue
	{
		friend class GraphicsDevice;
	private:
		CommandQueue(GraphicsDevice *owner) :
			mCmdMemory(4096, 4),
			mBufferMemory(MemoryPool::MEGABYTE * 4, 1)
		{
			mLastCmd = nullptr;
			mFirstCmd = nullptr;
			mDevice = owner;
		}

		~CommandQueue()
		{

		}

	public:
		// Allocates a new command.
		template<class T>
		inline T* alloc()
		{
			// Build up linked list.
			ICommand *cmd = mCmdMemory.malloc<T>();
			if (mLastCmd != nullptr)
				mLastCmd->next = cmd;
			else
				mFirstCmd = cmd;
			mLastCmd = cmd;
			return static_cast<T*>(cmd);
		}

		// Copies data for the command.
		void* memcpy(size_t size, void *data)
		{
			void *buffer = mBufferMemory.malloc(size);
			std::memcpy(buffer, data, size);
			return buffer;
		}

		inline ICommand* beginList()
		{
			return mFirstCmd;
		}

		inline void resetQueue()
		{
			mLastCmd = nullptr;
			mFirstCmd = nullptr;
			mCmdMemory.free();
			mBufferMemory.free();
		}

	private:
		MemoryPool mBufferMemory;
		MemoryPool mCmdMemory;
		ICommand *mLastCmd;
		ICommand *mFirstCmd;
		GraphicsDevice *mDevice;
	};
}

#endif