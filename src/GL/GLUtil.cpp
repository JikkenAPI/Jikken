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

#include "GL/GLUtil.hpp"
#include <iostream>

namespace Jikken
{
	namespace glutils
	{
		GLenum bufferUsageHintToGL(BufferUsageHint hint)
		{
			
			switch (hint)
			{
			case BufferUsageHint::eImmutable:
			case BufferUsageHint::eStatic:
				return GL_STATIC_DRAW;
			case BufferUsageHint::eDynamic:
				return GL_DYNAMIC_DRAW;
			default:
				return GL_INVALID_ENUM;
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
			default:
				return GL_INVALID_ENUM;
			}
		}

		GLenum layoutTypeToGL(VertexAttributeType type)
		{
			switch (type)
			{
			case VertexAttributeType::eFLOAT:
				return GL_FLOAT;
			default:
				return GL_INVALID_ENUM;
			}
		}

		GLenum shaderStageToGL(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::eVertex:
				return GL_VERTEX_SHADER;
			case ShaderStage::eFragment:
				return GL_FRAGMENT_SHADER;
			case ShaderStage::eGeometry:
				return GL_GEOMETRY_SHADER;
			default:
				return GL_INVALID_ENUM;
			}
		}

		GLenum drawPrimitiveToGL(PrimitiveType type)
		{
			switch (type)
			{
			case PrimitiveType::eTriangles:
				return GL_TRIANGLES;
			case PrimitiveType::eTriangleStrip:
				return GL_TRIANGLE_STRIP;
			case PrimitiveType::eLines:
				return GL_LINES;
			case PrimitiveType::eLineStrip:
				return GL_LINE_STRIP;
			default:
				return GL_INVALID_ENUM;
			}
		}

		GLenum blendStateToGL(BlendState state)
		{
			switch (state)
			{
			case BlendState::eSrcAlpha:
				return GL_SRC_ALPHA;
			case BlendState::eOneMinusSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			default:
				return GL_INVALID_ENUM;
			}
		}

		GLenum depthFuncToGL(DepthFunc func)
		{
			switch (func)
			{
			case DepthFunc::eAlways:
				return GL_ALWAYS;
			case DepthFunc::eEqual:
				return GL_EQUAL;
			case DepthFunc::eGreater:
				return GL_GREATER;
			case DepthFunc::eGreaterEqual:
				return GL_GEQUAL;
			case DepthFunc::eLess:
				return GL_LESS;
			case DepthFunc::eLessEqual:
				return GL_LEQUAL;
			case DepthFunc::eNever:
				return GL_NEVER;
			case DepthFunc::eNotEqual:
				return GL_NOTEQUAL;
			default:
				return GL_INVALID_ENUM;
			}
		}

		void printDeviceInfo()
		{
			std::printf("Vendor: %s\n", glGetString(GL_VENDOR));
			std::printf("Renderer: %s\n", glGetString(GL_RENDERER));
			std::printf("Version: %s\n", glGetString(GL_VERSION));
		}
	}
}
