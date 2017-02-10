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

#ifndef _JIKKEN_GRAPHICSDEVICE_HPP_
#define _JIKKEN_GRAPHICSDEVICE_HPP_

#include <vector>
#include <string>
#include "jikken/types.hpp"
#include "jikken/enums.hpp"
#include "jikken/structs.hpp"
#include "jikken/commands.hpp"
#include "jikken/commandQueue.hpp"

namespace Jikken
{
	class GraphicsDevice
	{
	public:
		virtual ~GraphicsDevice() {}

		virtual ShaderHandle createShader(const std::vector<ShaderDetails> &shaders) = 0;

		virtual BufferHandle createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, float *data) = 0;

		virtual LayoutHandle createVertexInputLayout(const std::vector<VertexInputLayout> &attributes) = 0;

		virtual VertexArrayHandle createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = 0) = 0;

		virtual void bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, int32_t index) = 0;

		virtual void deleteVertexInputLayout(LayoutHandle handle) = 0;

		virtual void deleteVAO(VertexArrayHandle handle) = 0;

		virtual void deleteBuffer(BufferHandle handle) = 0;

		virtual void deleteShader(ShaderHandle handle) = 0;
	};
}

#endif
