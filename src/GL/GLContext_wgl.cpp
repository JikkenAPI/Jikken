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

#include "GL/GLContext_wgl.hpp"

namespace Jikken
{
	GLContext::GLContext() :
         mHdc(nullptr),
         mHwnd(nullptr),
         mHrc(nullptr)
	{
	}

	GLContext::~GLContext()
	{
		clearCurrent();
		wglDeleteContext(mHrc);
		ReleaseDC(mHwnd, mHdc);
	}

    int32_t GLContext::_getPixelAttribs(const ContextConfig &contextConfig, int32_t *attribs)
	{
		int32_t i = 0;
		attribs[i++] = WGL_DRAW_TO_WINDOW_ARB;
		attribs[i++] = GL_TRUE;
		attribs[i++] = WGL_SUPPORT_OPENGL_ARB;
		attribs[i++] = GL_TRUE;
		attribs[i++] = WGL_PIXEL_TYPE_ARB;
		attribs[i++] = WGL_TYPE_RGBA_ARB;
		attribs[i++] = WGL_ACCELERATION_ARB;
		attribs[i++] = WGL_FULL_ACCELERATION_ARB;

		attribs[i++] = WGL_RED_BITS_ARB;
		attribs[i++] = 8;
		attribs[i++] = WGL_GREEN_BITS_ARB;
		attribs[i++] = 8;
		attribs[i++] = WGL_BLUE_BITS_ARB;
		attribs[i++] = 8;
		attribs[i++] = WGL_ALPHA_BITS_ARB;
		attribs[i++] = 8;
		attribs[i++] = WGL_DEPTH_BITS_ARB;
		attribs[i++] = contextConfig.depthBits;
		attribs[i++] = WGL_STENCIL_BITS_ARB;
		attribs[i++] = contextConfig.stencilBits;

		attribs[i++] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
        attribs[i++] = contextConfig.srgbEnabled ? GL_TRUE : GL_FALSE;

		if (contextConfig.msaaLevel > 0)
		{
			attribs[i++] = WGL_SAMPLE_BUFFERS_ARB;
			attribs[i++] = 1;
			attribs[i++] = WGL_SAMPLES_ARB;
			attribs[i++] = contextConfig.msaaLevel;
		}

		attribs[i++] = 0;
		return i;
	}

	bool GLContext::init(const ContextConfig &contextConfig, const NativeWindowData &windowData)
	{
		//check we got a window handle first
		if (!windowData.handle)
		{
			std::printf("User supplied window handle does not appear to be valid\n");
            return false;
		}

		//dummy window first to create old style gl context with and init glew
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = (WNDPROC)DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = 0;
		wc.hCursor = nullptr;
		wc.hbrBackground = 0;
		wc.lpszMenuName = 0;
		wc.lpszClassName = "dummy window class";
		wc.hIconSm = 0;

		RegisterClassEx(&wc);

		HWND tmpHWND = CreateWindowEx(WS_EX_APPWINDOW, "dummy window class", "", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 10, 10, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
		if (!tmpHWND)
		{
			std::printf("failed to create OpenGL temporary window\n");
			return false;
		}

		HDC tmpHDC = GetDC(tmpHWND);

		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cStencilBits = 8;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;

		if (!SetPixelFormat(tmpHDC, ChoosePixelFormat(tmpHDC, &pfd), &pfd))
		{
			std::printf("failed to set pixel format on OpenGL temporary window\n");
            DestroyWindow(tmpHWND);
            return false;
		}

		HGLRC tmpHGLRC = wglCreateContext(tmpHDC);

		if (!tmpHGLRC)
		{
			std::printf("failed to create temporary OpenGL rendering context\n");
            DestroyWindow(tmpHWND);
            return false;
		}

		if (!wglMakeCurrent(tmpHDC, tmpHGLRC))
		{
			std::printf("failed to make temporary OpenGL context current\n");
			wglDeleteContext(tmpHGLRC);
            DestroyWindow(tmpHWND);
            return false;
		}

		//init wgl extensions
		GLenum err = wglewInit();
		//cleanup temp resources
		wglMakeCurrent(tmpHDC, nullptr);
		wglDeleteContext(tmpHGLRC);
		ReleaseDC(tmpHWND, tmpHDC);
		DestroyWindow(tmpHWND);
		UnregisterClass("dummy window class", GetModuleHandle(nullptr));
		//check wglew init was ok
		if(err != GLEW_OK )
		{
			std::printf("failed to load WGL extensions: %s\n", glewGetErrorString(err));
            return false;
		}

		if (!WGLEW_ARB_create_context || !WGLEW_ARB_pixel_format)
		{
			std::printf("Required WGL extensions not supported");
			return false;
		}

		//create GL 3.3 context now
		mHwnd = static_cast<HWND>(windowData.handle);
		mHdc = GetDC(mHwnd);

		int32_t pixelAttribs[50];//raise this if more are needed in getPixelAttribs
		int32_t num = _getPixelAttribs(contextConfig, pixelAttribs);
		int32_t pixelFormat = 0;
		uint32_t numFormats;
		wglChoosePixelFormatARB(mHdc, pixelAttribs, nullptr, 1, &pixelFormat, &numFormats);

		if (!DescribePixelFormat(mHdc, pixelFormat, sizeof(pfd), &pfd))
		{
			std::printf("failed to retrieve pixel format for OpenGL window\n");
			return false;
		}

		if (!SetPixelFormat(mHdc, pixelFormat, &pfd))
		{
			std::printf("failed to set pixel format on OpenGL window\n");
            return false;
		}

		const GLint OGL_MAJOR = 3;
		const GLint OGL_MINOR = 3;

		int32_t debugFlag = 0;
		if(contextConfig.debugEnabled)
			debugFlag = WGL_CONTEXT_DEBUG_BIT_ARB;

		const int32_t glContextAttributes[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, OGL_MAJOR,
			WGL_CONTEXT_MINOR_VERSION_ARB, OGL_MINOR,
			WGL_CONTEXT_FLAGS_ARB, debugFlag,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		mHrc = wglCreateContextAttribsARB(mHdc, nullptr, glContextAttributes);

		if (!mHrc)
		{
            std::printf("Failed to create OpenGL rendering context\n");
            return false;
		}

		//make our new context current
		if (!wglMakeCurrent(mHdc, mHrc))
		{
			std::printf("Failed to make context current\n");
			return false;
		}

		//init GL extensions
		err = glewInit();
		if (err != GLEW_OK)
		{
			std::printf("failed to load GL extensions: %s\n", glewGetErrorString(err));
			return false;
		}

		//double check version is ok
		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		if (major < OGL_MAJOR || (major == OGL_MAJOR && minor < OGL_MINOR))
		{
			std::printf("OpenGL version 3.3 is required");
			return false;
		}
	
		//set swap interval
		setSwapInterval(contextConfig.vsyncEnabled ? 1 : 0);

		//happy days
		return true;
	}

	void GLContext::makeCurrent()
	{
		if (!wglMakeCurrent(mHdc, mHrc))
		{
			std::printf("Failed to make context current\n");
		}
	}

	void GLContext::clearCurrent()
	{
		if (!wglMakeCurrent(mHdc, nullptr))
		{
			std::printf("Failed to clear context\n");
		}
	}

	void GLContext::swapBuffers()
	{
		if (!SwapBuffers(mHdc))
		{
            std::printf("Failed to swap buffers\n");
		}
	}

	void GLContext::setSwapInterval(const int32_t val)
	{
		if (WGLEW_EXT_swap_control)
		{
			//nvidia drivers for some reason need the default framebuffer bound or wglSwapInterval fails
			GLint fboId;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fboId);
			if(fboId !=0)
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			if (WGLEW_EXT_swap_control_tear)
			{
				if (val == 1 || val == -1)
				{
					BOOL ret = wglSwapIntervalEXT(-1);

					if (!ret)
						wglSwapIntervalEXT(1);
				}
				else
				{
					wglSwapIntervalEXT(0);
				}
			}
			else
			{
				wglSwapIntervalEXT(val);
			}

			if(fboId != 0)
				glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		}
	}

}
