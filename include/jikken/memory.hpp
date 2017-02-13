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
#include <vector>

namespace Jikken
{
	/// A custom memory allocator that operates as if it was on a stack.
	/// Once a page fills up within the pool, a new page is generated.
	/// Page references to the next page are stored as the last 
	/// sizeof(uintptr_t) bytes on the current page.
	/// Once free() is called, all memory is considered to be reset.
	class PerFrameMemoryPool
	{
		typedef uint8_t* Page;

	public:
		const size_t PAGE_SIZE = 2048*8*4;

		const size_t PAGE_OFFSET = PAGE_SIZE - sizeof(Page);

		/// Gets the pointer to the next page of memory in the pool.
		/// @param page The memory page we are checking.
		/// @return a pointer to the next page, or 0x0 if one does not eixst.
	private:
		inline Page* getNextPageOffset(Page page)
		{
			return reinterpret_cast<Page*>(&page[PAGE_OFFSET]);
		}

	public:
		PerFrameMemoryPool()
		{
			// Allocate a page of memory.
			mMemory = new uint8_t[PAGE_SIZE];

			// The last bytes will hold a pointer to the next page of memory we will
			// allocate. Since we only want 1 page for now, just store 0x0
			Page *nextPage = getNextPageOffset(mMemory);
			*nextPage = nullptr;

			// Offset within our current page.
			mCurrentPtr = 0;

			// Pointer to our current page
			mCurrentPage = &mMemory;
		}

		~PerFrameMemoryPool()
		{
			// First we have to bulid the list of pages that we have to free.
			std::vector<Page*> pages{ &mMemory };
			Page *page = getNextPageOffset(mMemory);
			while (*page != nullptr)
			{
				pages.push_back(page);
				page = getNextPageOffset(*page);
			}

			// Now that we have our list of pages, we can free them.
			for (Page *mem : pages)
			{
				delete[] *mem;
			}
		}

		template<class T>
		T* malloc()
		{
			size_t size = sizeof(T);
			if (size + mCurrentPtr >= PAGE_OFFSET)
			{
				// See if we already have a page allocated.
				// If we do, we're golden. If not, just alloc another page!
				Page *next = getNextPageOffset(*mCurrentPage);
				if (*next == nullptr)
				{
					// Alloc new page.
					Page page = new uint8_t[PAGE_SIZE];

					// Assign the current page's next page ptr to this page.
					next = &page;

					// The last bytes will hold a pointer to the next page of memory we will
					// allocate. Set it to null since we are only allocating 1 page at a time.
					Page *pg = getNextPageOffset(page);
					*pg = nullptr;

					// Set current page
					mCurrentPage = &page;
				}
				else
				{
					// we already have a page allocated
					mCurrentPage = next;
				}

				// reset current offset of new page to beginning.
				mCurrentPtr = 0;
			}

			T *obj = reinterpret_cast<T*>(*mCurrentPage + mCurrentPtr);
			mCurrentPtr += static_cast<uint32_t>(size);

			// Note the use of placement new. It doesn't do any
			// allocation. It just calls the constructor of our
			// already allocated object.
			return new(obj) T();
		}

		void free()
		{
			// Reset the stack.
			mCurrentPtr = 0;
			mCurrentPage = &mMemory;
		}

		uint8_t* getCommandQueuePtr() const
		{
			return mMemory;
		}

	private:
		Page mMemory;
		uint32_t mCurrentPtr;
		Page *mCurrentPage;
	};
}

#endif