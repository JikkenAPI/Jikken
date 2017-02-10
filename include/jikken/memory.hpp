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

#ifndef _JIKKEN_MEMORY_HPP_
#define _JIKKEN_MEMORY_HPP_

#include <cstdint>

// TODO: IMPLEMENT MEMORY PAGING.

namespace Jikken
{
	/// A custom memory allocator that operates as if it was on a stack.
	/// Once free() is called, all memory is considered to be reset.
	class PerFrameMemoryPool
	{
		const size_t PAGE_SIZE = 2048;
	public:
		PerFrameMemoryPool()
		{
			// Allocate a page of memory.
			mMemory = new uint8_t[PAGE_SIZE];
			mCurrentPtr = 0;
		}

		~PerFrameMemoryPool()
		{
			// Deallocate page
			delete[] mMemory;
		}

		template<class T>
		T* malloc()
		{
			size_t size = sizeof(T);
			if (size + mCurrentPtr >= PAGE_SIZE)
			{
				// We've run out of ram. For now, fail.
				// TODO: alloc more.
				assert(false);
				abort();
			}

			T *obj = reinterpret_cast<T*>(mMemory + mCurrentPtr);
			mCurrentPtr += size;

			// Note the use of placement new. It doesn't do any
			// allocation. It just calls the constructor of our
			// already allocated object.
			return new(obj) T();
		}

		void free()
		{
			// Reset the stack.
			mCurrentPtr = 0;
		}

	private:
		uint8_t *mMemory;
		uint32_t mCurrentPtr;
	};
}

#endif