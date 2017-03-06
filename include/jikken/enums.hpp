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

#ifndef _JIKKEN_ENUMS_HPP_
#define _JIKKEN_ENUMS_HPP_

#include <cstdint>

namespace Jikken
{
	enum class GraphicsApi : uint8_t
	{
		eNull,
		eOpenGL,
		eDirect3D11,
		eVulkan
	};

	enum class BlendState : uint8_t
	{
		eZero = 0,
		eOne,
		eSrcColor,
		eOneMinusSrcColor,
		eSrcAlpha,
		eOneMinusSrcAlpha,
		eDstAlpha,
		eOneMinusDstAlpha,
		eDstColor,
		eOneMinusDstColor
	};

	enum class DepthFunc : uint8_t
	{
		eNever = 0,
		eAlways,
		eLess,
		eEqual,
		eNotEqual,
		eGreater,
		eLessEqual,
		eGreaterEqual
	};

	enum class CullFaceState : uint8_t
	{
		eFront = 0,
		eBack
	};

	enum class WindingOrderState : uint8_t
	{
		eCW = 0,
		eCCW
	};

	enum class PrimitiveType : uint8_t
	{
		eTriangles = 0,
		eTriangleStrip,
		eLines,
		eLineStrip
	};

	enum class ShaderStage : uint8_t
	{
		eVertex = 0,
		eFragment,
		eGeometry,
		eCompute
	};

	enum ClearBufferFlags : uint32_t
	{
		eColor = 1,
		eDepth = 2,
		eStencil = 4
	};

	enum class BufferType : uint8_t
	{
		eVertexBuffer = 0,
		eIndexBuffer,
		eConstantBuffer
	};

	enum class BufferUsageHint : uint8_t
	{
		eStaticDraw = 0,
		eDynamicDraw,
		eStreamDraw
	};

	enum VertexAttributeName : int32_t
	{
		ePOSITION = 0
	};

	enum VertexAttributeType : int32_t
	{
		eFLOAT = 0
	};
}

#endif