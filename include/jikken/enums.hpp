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

namespace Jikken
{
	enum class BlendState
	{
		eSrcAlpha = 0,
		eOneMinusSrcAlpha
	};

	enum class DepthFunc
	{
		eNever = 0,
		eAlways,
		eLess,
		eEqual,
		eGreater,
		eLessEqual,
		eGreaterEqual
	};

	enum class CullFaceState
	{
		eFront = 0,
		eBack
	};

	enum class WindingOrderState
	{
		eCW = 0,
		eCCW
	};

	enum class PrimitiveType
	{
		eTriangles = 0,
		eTriangleStrip,
		eLines,
		eLineStrip
	};

	enum class ShaderStage
	{
		eVertex = 0,
		eFragment,
		eGeometry,
		eCompute
	};

	enum class ClearBufferFlags
	{
		eColor = 1,
		eDepth = 2,
		eStencil = 4
	};

	enum class BufferType
	{
		eVertexBuffer = 0,
		eIndexBuffer,
		eConstantBuffer
	};

	enum class BufferUsageHint
	{
		eStaticDraw = 0,
		eDynamicDraw,
		eStreamDraw
	};
}

#endif