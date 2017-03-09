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

#ifndef _JIKKEN_STRUCTS_HPP_
#define _JIKKEN_STRUCTS_HPP_

#include <string>
#include "jikken/enums.hpp"

namespace Jikken
{
	//adapter information
	/*struct Adapter
	{
		GraphicsApi api; //render api for this adapater
		uint32_t index; //index of this adapater
		uint32_t memory; //adapter memory in megabytes
		std::string vendor; //vendor name
		//features
		ShaderModel shaderModel; //shader model supported
		bool discrete; //is this adapater a discrete or integrated adapter
	};*/

	// native window data
	struct NativeWindowData
	{
		void* handle;
		void* display;
		int32_t fbHeight;
		int32_t fbWidth;
	};

	// Render context configuration
	struct ContextConfig
	{
		uint32_t depthBits;
		uint32_t stencilBits;
		uint32_t msaaLevel;
		bool vsyncEnabled;
		bool srgbEnabled;
		bool debugEnabled;
		//set some sane defaults
		explicit ContextConfig(uint32_t depth=24, uint32_t stencil=8, uint32_t msaa=0,bool vsync=false,bool srgb=false,bool debug=false) :
           depthBits(depth),stencilBits(stencil),msaaLevel(msaa),vsyncEnabled(vsync),srgbEnabled(srgb), debugEnabled(debug){}
	};

	struct VertexInputLayout
	{
		VertexAttributeName attribute;
		int32_t componentSize;
		VertexAttributeType type;
		uint32_t stride;
		size_t offset;
	};

	struct ShaderDetails
	{
		std::string file;
		ShaderStage stage;
	};
}

#endif