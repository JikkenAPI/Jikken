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

#ifndef _JIKKEN_JIKKEN_HPP_
#define _JIKKEN_JIKKEN_HPP_

#include "jikken/enums.hpp"
#include "jikken/structs.hpp"
#include "jikken/commandQueue.hpp"
#include <vector>

namespace Jikken
{
	/////////////////////////////////////////////
	// Startup and shutdown
	/////////////////////////////////////////////

	// get list of available adapters
	//std::vector<Adapter> queryAdapters();

	//initialize jikken - if using null adapter, ContextConfig & NativeWindowData ignored
	//todo proper Adapter support 
	bool init(const GraphicsApi api, const ContextConfig &contextConfig, const NativeWindowData &windowData);
	//shutdown jikken
	void shutdown();

	/////////////////////////////////////////////
	// Graphics device functions
	/////////////////////////////////////////////

	//creation
	CommandQueue* createCommandQueue();
	ShaderHandle createShader(const std::vector<ShaderDetails> &shaders);
	BufferHandle createBuffer(BufferType type);
	LayoutHandle createVertexInputLayout(const std::vector<VertexInputLayout> &attributes);
	VertexArrayHandle createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = InvalidHandle);
	//deletion
	void deleteCommandQueue(CommandQueue *cmdQueue);
	void deleteVertexInputLayout(LayoutHandle handle);
	void deleteVAO(VertexArrayHandle handle);
	void deleteBuffer(BufferHandle handle);
	void deleteShader(ShaderHandle handle);
	//queue submission
	void submitCommandQueue(CommandQueue *queue);
	// todo - make command for this??
	void bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index);
	//present the frame
	void presentFrame();
	
	//window resize event
	void resize(const int32_t width,const int32_t height);
}

#endif