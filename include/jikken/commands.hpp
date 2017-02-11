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
#include "jikken/memory.hpp"

namespace Jikken
{
	enum CommandType : uint32_t
	{
		eAbstract = 0,
		eSetShader,
		eUpdateBuffer,
		eDraw,
		eDrawInstance,
		eClearBuffer,
		eBindVAO,
		eViewport,
		eBlendState,
		eDepthStencilState,
		eCullState
	};

	struct ICommand
	{
		friend class CommandQueue;
		friend class PerFrameMemoryPool;
		friend class GraphicsDevice;
		friend class GLGraphicsDevice;

		ICommand();
		CommandType commandType;

		// Restrict next pointer only for memory management. Not API.
	private:
		ICommand *next;
	};

	struct SetShaderCommand : public ICommand
	{
		SetShaderCommand();
		ShaderHandle handle;
	};

	struct UpdateBufferCommand : public ICommand
	{
		UpdateBufferCommand();

		BufferHandle buffer;
		size_t offset;
		size_t dataSize;
		union
		{
			float *fData;
			uint16_t *sData;
			void *data;
		};
	};

	struct DrawCommand : public ICommand
	{
		DrawCommand();

		PrimitiveType primitive;
		uint32_t start;
		uint32_t count;
	};

	struct DrawInstanceCommand : public ICommand
	{
		DrawInstanceCommand();

		PrimitiveType primitive;
		uint32_t start;
		uint32_t count;
		uint32_t instancedCount;
	};

	struct ClearBufferCommand : public ICommand
	{
		ClearBufferCommand();
		uint32_t flag;
	};

	struct BindVAOCommand : public ICommand
	{
		BindVAOCommand();

		VertexArrayHandle vertexArray;
	};

	struct ViewportCommand : public ICommand
	{
		ViewportCommand();

		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	struct BlendStateCommand : public ICommand
	{
		BlendStateCommand();

		bool enabled;
		BlendState source;
		BlendState dest;
	};

	// Right now, just depth. TODO: add stencil
	struct DepthStencilStateCommand : public ICommand
	{
		DepthStencilStateCommand();

		bool depthEnabled;
		bool depthWrite;
		DepthFunc depthFunc;
	};

	struct CullStateCommand : public ICommand
	{
		CullStateCommand();

		bool enabled;
		CullFaceState face;
		WindingOrderState state;
	};
}
#endif