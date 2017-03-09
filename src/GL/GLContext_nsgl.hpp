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
#ifndef JIKKEN_GL_NSGLCONTEXT_H
#define JIKKEN_GL_NSGLCONTEXT_H

#include "jikken/enums.hpp"
#include "jikken/structs.hpp"

#ifdef __OBJC__
@class NSOpenGLView;
#else
class NSOpenGLView;
#endif

namespace Jikken
{
	class GLContext
	{
	public:
		GLContext();
         ~GLContext();

         bool init(const ContextConfig &contextConfig, const NativeWindowData &windowData);
         void makeCurrent();
		 void clearCurrent();
         void swapBuffers();
		 void setSwapInterval(const int32_t val);

	private:
		//functions
		int32_t _getPixelAttribs(const ContextConfig &contextConfig, int32_t *attribs);
		
		NSOpenGLView *mView;
	};
}

#endif