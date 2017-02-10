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

#include "GLUtil.hpp"

namespace Jikken
{
	GLenum bufferUsageHintToGL(BufferUsageHint hint)
	{
		switch (hint)
		{
		case BufferUsageHint::eStaticDraw:
			return GL_STATIC_DRAW;
		case BufferUsageHint::eDynamicDraw:
			return GL_DYNAMIC_DRAW;
		case BufferUsageHint::eStreamDraw:
			return GL_STREAM_DRAW;
		}
	}

	GLenum bufferTypeToGL(BufferType type)
	{
		switch (type)
		{
		case BufferType::eVertexBuffer:
			return GL_ARRAY_BUFFER;
		case BufferType::eIndexBuffer:
			return GL_ELEMENT_ARRAY_BUFFER;
		case BufferType::eConstantBuffer:
			return GL_UNIFORM_BUFFER;
		}
	}

	GLenum layoutTypeToGL(VertexAttributeType type)
	{
		switch (type)
		{
		case VertexAttributeType::eFLOAT:
			return GL_FLOAT;
		}
	}
}