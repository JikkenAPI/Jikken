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
		CommandQueue()
		{

		}

		~CommandQueue()
		{

		}

	public:
		template<class T>
		inline T* alloc()
		{
			return mMemory.malloc<T>();
		}

		inline ICommand* beginList()
		{
			return reinterpret_cast<ICommand*>(mMemory.getCommandQueuePtr());
		}

		inline void resetQueue()
		{
			mMemory.free();
		}

	private:
		PerFrameMemoryPool mMemory;
		ICommand *mCurrentPtr;
	};
}

#endif