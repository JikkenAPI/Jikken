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

#ifndef _JIKKEN_COMMANDS_HPP_
#define _JIKKEN_COMMANDS_HPP_

#include "jikken/types.hpp"
#include "jikken/enums.hpp"

namespace Jikken
{
	enum CommandType : uint8_t
	{
		eSetShader=0,
		eBeginFrame,
		eUpdateBuffer,
		eReallocBuffer,
		eDraw,
		eDrawInstance,
		eClearBuffer,
		eBindVAO,
		eViewport,
		eBlendState,
		eDepthStencilState,
		eCullState,
		eFinishQueue //special value, doesn't need command struct
	};

	struct SetShaderCommand 
	{
		ShaderHandle handle;
	};

	struct BeginFrameCommand
	{
		uint32_t clearFlag;
		float clearColor[4];
		float depth;
		int32_t stencil;
	};

	struct UpdateBufferCommand
	{
		BufferHandle buffer;
		size_t offset;
		size_t dataSize;
		void *data;
	};

	struct ReallocBufferCommand
	{
		BufferHandle buffer;
		size_t stride;
		size_t count;
		void *data;
		BufferUsageHint hint;
	};

	struct DrawCommand
	{
		PrimitiveType primitive;
		uint32_t start;
		uint32_t count;
	};

	struct DrawInstanceCommand
	{
		PrimitiveType primitive;
		uint32_t start;
		uint32_t count;
		uint32_t instancedCount;
	};

	struct ClearBufferCommand
	{
		uint32_t flag;
	};

	struct BindVAOCommand
	{
		VertexArrayHandle vertexArray;
	};

	struct ViewportCommand
	{
		int16_t x;
		int16_t y;
		int16_t width;
		int16_t height;
	};

	struct BlendStateCommand
	{
		bool enabled;
		BlendState source;
		BlendState dest;
	};

	// Right now, just depth. TODO: add stencil
	struct DepthStencilStateCommand 
	{
		bool depthEnabled;
		bool depthWrite;
		DepthFunc depthFunc;
	};

	struct CullStateCommand
	{
		bool enabled;
		CullFaceState face;
		WindingOrderState state;
	};
}
#endif