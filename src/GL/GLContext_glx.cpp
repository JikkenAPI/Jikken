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

#include "GL/GLContext_glx.hpp"

namespace Jikken
{
    GLContext::GLContext() :
        mContext(nullptr),
        mDisplay(nullptr)
    {
    }

    GLContext::~GLContext()
    {
        if(mDisplay)
            glXMakeCurrent( mDisplay, 0, nullptr );
        if(mContext && mDisplay)
            glXDestroyContext( mDisplay, mContext );
    }

    int32_t GLContext::_getPixelAttribs(const ContextConfig &contextConfig, int32_t *attribs)
    {
        int32_t i = 0;
        attribs[i++] = GLX_X_RENDERABLE;
        attribs[i++] = True;
        attribs[i++] = GLX_DRAWABLE_TYPE;
        attribs[i++] = GLX_WINDOW_BIT;
        attribs[i++] = GLX_RENDER_TYPE;
        attribs[i++] = GLX_RGBA_BIT;

        attribs[i++] = GLX_RED_SIZE;
        attribs[i++] = 8;
        attribs[i++] = GLX_GREEN_SIZE;
        attribs[i++] = 8;
        attribs[i++] = GLX_BLUE_SIZE;
        attribs[i++] = 8;
        attribs[i++] = GLX_ALPHA_SIZE;
        attribs[i++] = 8;

        attribs[i++] = GLX_DOUBLEBUFFER;
        attribs[i++] = True;

        attribs[i++] = GLX_DEPTH_SIZE;
        attribs[i++] = contextConfig.depthBits;

        attribs[i++] = GLX_STENCIL_SIZE;
        attribs[i++] = contextConfig.stencilBits;

        attribs[i++] = GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB;
        attribs[i++] = contextConfig.srgbEnabled ? True : False;

        if(contextConfig.msaaLevel)
        {
            attribs[i++] = GLX_SAMPLE_BUFFERS_ARB;
            attribs[i++] = 1;
            attribs[i++] = GLX_SAMPLES_ARB;
            attribs[i++] = contextConfig.msaaLevel;
        }

        if(GLXEW_EXT_visual_info)
        {
            attribs[i++] = GLX_X_VISUAL_TYPE;
            attribs[i++] = GLX_TRUE_COLOR;
        }

        attribs[i++] = None;
        return i;
    }

    GLXFBConfig GLContext::_getBestFBConfig(GLXFBConfig *config,const int32_t fbCount)
    {
        int32_t best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
        for (int32_t i=0; i<fbCount; ++i)
        {
            XVisualInfo *vi = glXGetVisualFromFBConfig( mDisplay, config[i] );
            if ( vi )
            {
                int32_t samp_buf, samples;
                glXGetFBConfigAttrib( mDisplay, config[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                glXGetFBConfigAttrib( mDisplay, config[i], GLX_SAMPLES, &samples  );

                if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
                    best_fbc = i, best_num_samp = samples;
                if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
                    worst_fbc = i, worst_num_samp = samples;
            }
            XFree( vi );
        }

        if(best_fbc)
            return config[best_fbc];
        else
            return config[0]; //something wrong here, return the first
    }

    bool GLContext::init(const ContextConfig &contextConfig, const NativeWindowData &windowData)
    {
        if (!windowData.handle || !windowData.display)
        {
            std::printf("User supplied window data does not appear to be valid\n");
            return false;
        }

        mDisplay = static_cast<Display*>(windowData.display);
        mWindow = (uintptr_t)windowData.handle;
        int32_t defaultScreen = DefaultScreen(mDisplay);



        return false;
    }

    void GLContext::makeCurrent()
    {
        if(!glXMakeCurrent(mDisplay,mWindow, mContext))
        {
            std::printf("Failed to make context current\n");
        }
    }

   	void GLContext::clearCurrent()
	{
        if(!glXMakeCurrent(mDisplay, 0, 0))
        {
            std::printf("Failed to clear context\n");
        }
	}

    void GLContext::swapBuffers()
    {
        glXSwapBuffers( mDisplay,mWindow );
    }

   void GLContext::setSwapInterval(const int32_t val)
   {
       if (GLXEW_EXT_swap_control)
       {
          glXSwapIntervalEXT(mDisplay,mWindow,val);
       }
   }

}
