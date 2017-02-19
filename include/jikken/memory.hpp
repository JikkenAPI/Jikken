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

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <vector>

namespace Jikken
{
	class MemoryPage
	{
	public:
		typedef uint8_t* Page;

		explicit MemoryPage(size_t pageSize)
		{
			mMemory = new uint8_t[pageSize];
			mPageSize = pageSize;
			mPointer = 0;
		}

		~MemoryPage()
		{
			delete[] mMemory;
		}

		/// malloc an object of size T to the page.
		/// @return A pointer to an object of type T, or nullptr if there wasn't sufficient space.
		template<class T>
		T* malloc()
		{
			size_t size = sizeof(T);
			if (size + mPointer > mPageSize)
				return nullptr;

			T *obj = reinterpret_cast<T*>(mMemory + mPointer);
			mPointer += static_cast<int32_t>(size);

			// Note the use of placement new. It doesn't do any
			// allocation. It just calls the constructor of our
			// already allocated object.
			return new(obj) T();
		}

		void* malloc(size_t size)
		{
			if (size + mPointer > mPageSize)
				return nullptr;
			void *mem = mMemory + mPointer;
			mPointer += static_cast<int32_t>(size);
			return mem;
		}

		inline void free()
		{
			mPointer = 0;
		}

	private:
		Page mMemory;
		size_t mPageSize;
		int32_t mPointer;
	};

	/// A custom memory allocator that operates as if it was on a stack.
	/// Once a page fills up within the pool, a new page is generated.
	/// Page references to the next page are stored as the last
	/// sizeof(uintptr_t) bytes on the current page.
	/// Once free() is called, all memory is considered to be reset.
	class MemoryPool
	{
	public:
		const static size_t MEGABYTE = 1048576;

		explicit MemoryPool(size_t pageSize, int32_t numDefaultPages)
		{
			for (int32_t i = 0; i < numDefaultPages; ++i)
				mPages.push_back(new MemoryPage(pageSize));
			mCurrentPage = 0;
			mPageSize = pageSize;
		}

		~MemoryPool()
		{
			for (auto page : mPages)
				delete page;
		}

		template<class T>
		T* malloc()
		{
#ifdef _DEBUG
			if (sizeof(T) > mPageSize)
			{
				printf("Need to increase stack frame. sizeof(T) is %d while pageSize is %zd\n", static_cast<int32_t>(sizeof(T)), mPageSize);
				assert(false);
			}
#endif

			T* obj = mPages[mCurrentPage]->malloc<T>();
			if (obj == nullptr)
			{
				// Increase to a new page. If we need another page, just alloc another one.
				++mCurrentPage;
				if (mCurrentPage == mPages.size())
					mPages.push_back(new MemoryPage(mPageSize));
				obj = mPages[mCurrentPage]->malloc<T>();
			}
			return obj;
		}


		void* malloc(size_t size)
		{
#ifdef _DEBUG
			if (size > mPageSize)
			{
				printf("Need to increase stack frame. sizeof(T) is %d while pageSize is %d\n", static_cast<int32_t>(size), static_cast<int32_t>(mPageSize));
				assert(false);
			}
#endif

			void *mem = mPages[mCurrentPage]->malloc(size);
			if (mem == nullptr)
			{
				// Increase to a new page. If we need another page, just alloc another one.
				++mCurrentPage;
				if (mCurrentPage == mPages.size())
					mPages.push_back(new MemoryPage(mPageSize));
				mem = mPages[mCurrentPage]->malloc(size);
			}
			return mem;
		}

		void free()
		{
			for (int32_t i = 0; i <= mCurrentPage; ++i)
				mPages[i]->free();
			mCurrentPage = 0;
		}

	private:

		std::vector<MemoryPage*> mPages;
		int32_t mCurrentPage;
		size_t mPageSize;
	};
}

#endif
