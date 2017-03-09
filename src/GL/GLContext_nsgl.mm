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

#include "GL/GLContext_nsgl.hpp"
#import <OpenGL/OpenGL.h>
#import <AppKit/AppKit.h>

namespace Jikken
{
	GLContext::GLContext()
	{
		mView = nil;
	}

	GLContext::~GLContext()
	{
		clearCurrent();
		
		[mView dealloc];
	}

	int32_t GLContext::_getPixelAttribs(const ContextConfig &contextConfig, int32_t *attribs)
	{
		int32_t i = 0;
		NSOpenGLPixelFormatAttribute *nsAttributeList = reinterpret_cast<NSOpenGLPixelFormatAttribute*>(attribs);
		
		// First thing's first, set up profile informatino
		nsAttributeList[i++] = NSOpenGLPFAOpenGLProfile;
		nsAttributeList[i++] = NSOpenGLProfileVersion3_2Core;
		
		// Next, fill in other attributes
		nsAttributeList[i++] = NSOpenGLPFAColorSize;
		nsAttributeList[i++] = 24;
		nsAttributeList[i++] = NSOpenGLPFAAlphaSize;
		nsAttributeList[i++] = 8;
		nsAttributeList[i++] = NSOpenGLPFADepthSize;
		nsAttributeList[i++] = contextConfig.depthBits;
		nsAttributeList[i++] = NSOpenGLPFAStencilSize;
		nsAttributeList[i++] = contextConfig.stencilBits;
		
		// No software renderer, and make sure we are double buffered.
		nsAttributeList[i++] = NSOpenGLPFADoubleBuffer;
		nsAttributeList[i++] = NSOpenGLPFAAccelerated;
		nsAttributeList[i++] = NSOpenGLPFANoRecovery;
		
		if (contextConfig.msaaLevel)
		{
			nsAttributeList[i++] = NSOpenGLPFASampleBuffers;
			nsAttributeList[i++] = 1;
			
			nsAttributeList[i++] = NSOpenGLPFASamples;
			nsAttributeList[i++] = contextConfig.msaaLevel;
		}

		// Finish attribute stack.
		nsAttributeList[i++] = 0;
		
		return i;
	}

	bool GLContext::init(const ContextConfig &contextConfig, const NativeWindowData &windowData)
	{
		if (!windowData.handle)
		{
			std::printf("User supplied window data does not appear to be valid\n");
			return false;
		}
		
		NSOpenGLPixelFormatAttribute pixelAttribs[50];//raise this if more are needed in _getPixelAttribs
		_getPixelAttribs(contextConfig, reinterpret_cast<int32_t*>(pixelAttribs));
		
		// Create pixel format.
		NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelAttribs];

		NSWindow *window = static_cast<NSWindow*>(windowData.handle);
		
		// create NSOpenGLView and set it to window.
		mView = [[NSOpenGLView alloc] initWithFrame:[window frame] pixelFormat:pixelFormat];
		NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
		[mView setOpenGLContext:context];

		_addViewToWindow(windowData);
		
		makeCurrent();
		
		//set swap interval
		setSwapInterval(contextConfig.vsyncEnabled ? 1 : 0);

		//happy days
		return true;
	}

	void GLContext::makeCurrent()
	{
		[[mView openGLContext] makeCurrentContext];
	}

	void GLContext::clearCurrent()
	{
		[NSOpenGLContext clearCurrentContext];
	}

	void GLContext::swapBuffers()
	{
		[[mView openGLContext] flushBuffer];
	}

	void GLContext::setSwapInterval(const int32_t val)
	{
		[[mView openGLContext] setValues:&val forParameter:NSOpenGLCPSwapInterval];
	}

	/// Code reference from BGFX for adding views/subviews properly to NSWindow.
	///
	/// Copyright 2010-2017 Branimir Karadzic. All rights reserved.
	///
	/// https://github.com/bkaradzic/bgfx
	///
	/// Redistribution and use in source and binary forms, with or without modification,
	/// are permitted provided that the following conditions are met:
	///
	///    1. Redistributions of source code must retain the above copyright notice, this
	///       list of conditions and the following disclaimer.
	///
	///    2. Redistributions in binary form must reproduce the above copyright notice,
	///       this list of conditions and the following disclaimer in the documentation
	///       and/or other materials provided with the distribution.
	///
	/// THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS OR
	/// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	/// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
	/// SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	/// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	/// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	/// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
	/// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
	/// OF THE POSSIBILITY OF SUCH DAMAGE.
	///
	/// https://github.com/bkaradzic/bgfx/blob/master/LICENSE
	void GLContext::_addViewToWindow(const NativeWindowData &windowData)
	{
		NSWindow *window = static_cast<NSWindow*>(windowData.handle);
		
		if ([window contentView] != nil)
		{
			[mView setAutoresizingMask:(
				NSViewHeightSizable |
				NSViewWidthSizable |
				NSViewMinXMargin |
				NSViewMaxXMargin |
				NSViewMinYMargin |
				NSViewMaxYMargin
			 )];
			[[window contentView] addSubview:mView];
		}
		else
		{
			[window setContentView:mView];
		}
	}
}
