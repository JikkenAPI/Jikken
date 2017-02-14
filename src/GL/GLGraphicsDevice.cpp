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

#include <cassert>
#include <cstdlib>
#include "GL/GLGraphicsDevice.hpp"
#include "GL/GLUtil.hpp"
//just temp
#include <GLFW/glfw3.h>

namespace Jikken
{
	void checkGLErrors()
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			printf("GL error: %i\n", err);
		}
	}

	GLGraphicsDevice::GLGraphicsDevice()
	{
		mBufferHandle = 0;
		mVertexArrayHandle = 0;
		mShaderHandle = 0;
		mLayoutHandle = 0;
		mCurrentVAO = 0;
		mWindowHandle = nullptr;

		mStateCache.blend.firstSet = true;
		mStateCache.depthStencil.firstSet = true;
		mStateCache.cull.firstSet = true;

		glGenVertexArrays(1, &mGlobalVAO);
		glBindVertexArray(mGlobalVAO);
	}

	GLGraphicsDevice::~GLGraphicsDevice()
	{
		glDeleteVertexArrays(1, &mGlobalVAO);
	}

	//todo
	bool GLGraphicsDevice::init(void *glfwWinHandle)
	{
		mWindowHandle = static_cast<GLFWwindow*>(glfwWinHandle);
		return true;
	}

	ShaderHandle GLGraphicsDevice::createShader(const std::vector<ShaderDetails> &shaders)
	{
		GLuint program;
		std::vector<GLuint> shaderAttachments;

		// Grab a GL attachment from each detail
		for (const ShaderDetails &details : shaders)
		{
			FILE *file = fopen(details.file.c_str(), "r");
			if (file == nullptr)
			{
				// TODO: Do something better than abort.
				printf("Unable to open %s for reading! Aborting!", details.file.c_str());
				abort();
			}

			// get file size
			long fileSize;
			fseek(file, 0, SEEK_END);
			fileSize = ftell(file);
			rewind(file);

			// read into buffer
			char *buffer = new char[fileSize + 1];
			memset(buffer, 0, fileSize + 1);
			fread(buffer, 1, static_cast<size_t>(fileSize), file);
			fclose(file);

			GLuint attachment = glCreateShader(shaderStageToGL(details.stage));
			glShaderSource(attachment, 1, &buffer, 0);
			glCompileShader(attachment);

			// delete buffer
			delete[] buffer;

			// Check for errors in uploading and compiling the shader.
			GLint success;
			glGetShaderiv(attachment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				// TODO: do something better than abort.
				// TODO: query error log length.
				const int LENGTH = 2048;
				GLchar info[LENGTH];
				glGetShaderInfoLog(attachment, LENGTH, 0, info);
				printf("Shader %s compile error.\n", details.file.c_str());
				printf(info);
				abort();
			}

			shaderAttachments.push_back(attachment);
		}

		// Create the program. A program is made up of one or more attachments.
		// When we link it, we can then free attachments.
		program = glCreateProgram();
		for (GLuint attachment : shaderAttachments)
			glAttachShader(program, attachment);
		glLinkProgram(program);

		// Check to see if the shader linked.
		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint length;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			char *msg = new char[length + 1];
			memset(msg, 0, length + 1);
			glGetProgramInfoLog(program, length, &length, msg);
			printf("Failed to link shader!\n");
			printf(msg);
			delete[] msg;

			// TODO: come up with better error handling.
			abort();
		}

		// Once it is linked, we can delete the attachments.
		for (GLuint attachment : shaderAttachments)
			glDeleteShader(attachment);

		checkGLErrors();

		ShaderHandle handle = mShaderHandle++;
		mShaderToGL[handle] = { program };
		return handle;
	}

	BufferHandle GLGraphicsDevice::createBuffer(BufferType type, BufferUsageHint hint, size_t dataSize, float *data)
	{
		BufferHandle handle = mBufferHandle++;
		GLuint buffer;

		glGenBuffers(1, &buffer);
		glBindBuffer(bufferTypeToGL(type), buffer);
		glBufferData(bufferTypeToGL(type), dataSize, data, bufferUsageHintToGL(hint));
		checkGLErrors();

		mBufferToGL[handle] = { type, buffer };
		return handle;
	}

	LayoutHandle GLGraphicsDevice::createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{
#ifdef _DEBUG
		if (attributes.size() == 0)
		{
			// We need at least 1 attribute.
			assert(false);
		}
#endif

		// Since the VAO handles layout in GL, we just copy it and store it.
		LayoutHandle layout = mLayoutHandle++;
		for (const VertexInputLayout &attr : attributes)
			mLayoutToGL[layout].push_back(attr);
		return layout;
	}

	VertexArrayHandle GLGraphicsDevice::createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer)
	{
#ifdef _DEBUG
		if (mBufferToGL[vertexBuffer].type != BufferType::eVertexBuffer)
			assert(false);
		if (indexBuffer != InvalidHandle && mBufferToGL[indexBuffer].type != BufferType::eIndexBuffer)
			assert(false);
#endif
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		{
			// Bind Vertex Buffer
			glBindBuffer(GL_ARRAY_BUFFER, mBufferToGL[vertexBuffer].buffer);

			// Index buffer is optional. We can draw without index buffers in OpenGL.
			if (indexBuffer != InvalidHandle)
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferToGL[indexBuffer].buffer);

			// Bind input layout.
			const std::vector<VertexInputLayout> &layouts = mLayoutToGL[layout];
			for (const VertexInputLayout &attr : layouts)
			{
				glEnableVertexAttribArray(attr.attribute);
				glVertexAttribPointer(
					attr.attribute, 
					attr.componentSize, 
					layoutTypeToGL(attr.type), 
					GL_FALSE, 
					attr.stride, 
					reinterpret_cast<void*>(attr.offset)
				);
			}
		}
		// Now bind global VAO. The above will now work for "vao"
		glBindVertexArray(mGlobalVAO);
		
		checkGLErrors();

		VertexArrayHandle handle = mVertexArrayHandle++;
		mVertexArrayToGL[handle] = { vertexBuffer, indexBuffer, layout, vao };
		return handle;
	}

	void GLGraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, const char *name, int32_t index)
	{
#ifdef _DEBUG
		if (mBufferToGL[cBuffer].type != BufferType::eConstantBuffer)
			assert(false);
#endif
		// Bind the uniform block to the shader program at index
		GLuint glIndex = glGetUniformBlockIndex(mShaderToGL[shader].program, name);
		glUniformBlockBinding(mShaderToGL[shader].program, glIndex, index);
		glBindBufferBase(GL_UNIFORM_BUFFER, index, mBufferToGL[cBuffer].buffer);
		checkGLErrors();
	}

	void GLGraphicsDevice::deleteVertexInputLayout(LayoutHandle handle)
	{
		// Nothing to delete in side GL, the handle object for us is just simply
		// a vector of attributes since the VAO handles everything.
		mLayoutToGL.erase(handle);
	}

	void GLGraphicsDevice::deleteVAO(VertexArrayHandle handle)
	{
		glDeleteVertexArrays(1, &mVertexArrayToGL[handle].vao);
		mVertexArrayToGL.erase(handle);
	}

	void GLGraphicsDevice::deleteBuffer(BufferHandle handle)
	{
		glDeleteBuffers(1, &mBufferToGL[handle].buffer);
		mBufferToGL.erase(handle);
	}

	void GLGraphicsDevice::deleteShader(ShaderHandle handle)
	{
		glDeleteProgram(mShaderToGL[handle].program);
		mShaderToGL.erase(handle);
	}

	void GLGraphicsDevice::submitCommandQueue(CommandQueue *cmdQueue)
	{
		// Process all commands and then clear the queue so it can be filled again.
		for (ICommand *cmd = cmdQueue->beginList(); cmd != nullptr; cmd = cmd->next)
			_processCmd(cmd);
		cmdQueue->resetQueue();
	}

	void GLGraphicsDevice::_processCmd(ICommand *cmd)
	{
		switch (cmd->commandType)
		{
		case CommandType::eSetShader:
			_setShaderCmd(static_cast<SetShaderCommand*>(cmd));
			break;
		case CommandType::eUpdateBuffer:
			_updateBufferCmd(static_cast<UpdateBufferCommand*>(cmd));
			break;
		case CommandType::eReallocBuffer:
			_reallocBufferCmd(static_cast<ReallocBufferCommand*>(cmd));
			break;
		case CommandType::eDraw:
			_drawCmd(static_cast<DrawCommand*>(cmd));
			break;
		case CommandType::eDrawInstance:
			_drawInstanceCmd(static_cast<DrawInstanceCommand*>(cmd));
			break;
		case CommandType::eClearBuffer:
			_clearBufferCmd(static_cast<ClearBufferCommand*>(cmd));
			break;
		case CommandType::eBindVAO:
			_bindVAOCmd(static_cast<BindVAOCommand*>(cmd));
			break;
		case CommandType::eViewport:
			_viewportCmd(static_cast<ViewportCommand*>(cmd));
			break;
		case CommandType::eBlendState:
			_blendStateCmd(static_cast<BlendStateCommand*>(cmd));
			break;
		case CommandType::eDepthStencilState:
			_depthStencilStateCmd(static_cast<DepthStencilStateCommand*>(cmd));
			break;
		case CommandType::eCullState:
			_cullStateCmd(static_cast<CullStateCommand*>(cmd));
			break;
		default:
			printf("Invalid command to process in GLGraphicsDevice: %i", cmd->commandType);
			break;
		}
	}

	void GLGraphicsDevice::_setShaderCmd(SetShaderCommand *cmd)
	{
		glUseProgram(mShaderToGL[cmd->handle].program);
		checkGLErrors();
	}

	void GLGraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{
		GLBuffer buffer = mBufferToGL[cmd->buffer];
		glBindBuffer(bufferTypeToGL(buffer.type), buffer.buffer);
		glBufferSubData(bufferTypeToGL(buffer.type), cmd->offset, cmd->dataSize, cmd->data);
		checkGLErrors();
	}

	void GLGraphicsDevice::_reallocBufferCmd(ReallocBufferCommand *cmd)
	{
		GLBuffer buffer = mBufferToGL[cmd->buffer];
		glBindBuffer(bufferTypeToGL(buffer.type), buffer.buffer);
		glBufferData(
			bufferTypeToGL(buffer.type), 
			cmd->stride * cmd->count, 
			cmd->data, 
			bufferUsageHintToGL(cmd->hint)
		);
		checkGLErrors();
	}

	void GLGraphicsDevice::_drawCmd(DrawCommand *cmd)
	{
		GLenum primitive = drawPrimitiveToGL(cmd->primitive);

		// Check if we are using indexed drawing.
		if (mVertexArrayToGL[mCurrentVAO].ibo == InvalidHandle)
		{
			glDrawArrays(primitive, cmd->start, cmd->count);
		}
		else
		{
#ifdef _MSC_VER
// warning C4312: 'reinterpret_cast': conversion from 'uint32_t' to 'void *' of greater size
#pragma warning(push)
#pragma warning(disable : 4312)
#endif
			glDrawElements(primitive, cmd->count, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(cmd->start));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		}
		checkGLErrors();
	}

	void GLGraphicsDevice::_drawInstanceCmd(DrawInstanceCommand *cmd)
	{
		GLenum primitive = drawPrimitiveToGL(cmd->primitive);

		// Check if we are using indexed drawing.
		if (mVertexArrayToGL[mCurrentVAO].ibo == InvalidHandle)
		{
			glDrawArraysInstanced(primitive, cmd->start, cmd->count, cmd->instancedCount);
		}
		else
		{
#ifdef _MSC_VER
// warning C4312: 'reinterpret_cast': conversion from 'uint32_t' to 'void *' of greater size
#pragma warning(push)
#pragma warning(disable : 4312)
#endif
			glDrawElementsInstanced(primitive, cmd->count, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(cmd->start), cmd->instancedCount);
#ifdef _MSC_VER
#pragma warning(pop)
#endif		
		}
	}

	void GLGraphicsDevice::_clearBufferCmd(ClearBufferCommand *cmd)
	{
		uint32_t flag = 0x0;
		if (cmd->flag & ClearBufferFlags::eColor)
			flag |= GL_COLOR_BUFFER_BIT;
		if (cmd->flag & ClearBufferFlags::eDepth)
			flag |= GL_DEPTH_BUFFER_BIT;
		if (cmd->flag & ClearBufferFlags::eStencil)
			flag |= GL_STENCIL_BUFFER_BIT;
		glClear(flag);
		checkGLErrors();
	}

	void GLGraphicsDevice::_bindVAOCmd(BindVAOCommand *cmd)
	{
		mCurrentVAO = cmd->vertexArray;
		const GLVAO &vao = mVertexArrayToGL[mCurrentVAO];
		glBindVertexArray(vao.vao);
		checkGLErrors();
	}

	void GLGraphicsDevice::_viewportCmd(ViewportCommand *cmd)
	{
		glViewport(cmd->x, cmd->y, cmd->width, cmd->height);
	}

	void GLGraphicsDevice::_blendStateCmd(BlendStateCommand *cmd)
	{
		// If this is the first time we are setting the state block,
		// update everything.
		bool first = mStateCache.blend.firstSet;

		// First toggle blending.
		if (first || cmd->enabled != mStateCache.blend.blendStateEnabled)
		{
			if (cmd->enabled)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);
			mStateCache.blend.blendStateEnabled = cmd->enabled;
		}

		// Next update the kind of blending we want to perform.
		if (first || (cmd->source != mStateCache.blend.source || cmd->dest != mStateCache.blend.dest))
		{
			glBlendFunc(blendStateToGL(cmd->source), blendStateToGL(cmd->dest));
			mStateCache.blend.source = cmd->source;
			mStateCache.blend.dest = cmd->dest;
		}

		// We've set it at least once.
		mStateCache.blend.firstSet = false;
	}

	void GLGraphicsDevice::_depthStencilStateCmd(DepthStencilStateCommand *cmd)
	{
		// If this is the first time we are setting the state block,
		// update everything.
		bool first = mStateCache.depthStencil.firstSet;

		// Enables/Disables depth test.
		if (first || (cmd->depthEnabled != mStateCache.depthStencil.depthEnabled))
		{
			if (cmd->depthEnabled)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
			mStateCache.depthStencil.depthEnabled = cmd->depthEnabled;
		}

		// Enables/Disables writing to the depth buffer.
		if (first || (cmd->depthWrite != mStateCache.depthStencil.depthWrite))
		{
			glDepthMask(cmd->depthWrite);
			mStateCache.depthStencil.depthWrite = cmd->depthWrite;
		}

		// Sets depth function.
		if (first || (cmd->depthFunc != mStateCache.depthStencil.depthFunc))
		{
			glDepthFunc(depthFuncToGL(cmd->depthFunc));
			mStateCache.depthStencil.depthFunc = cmd->depthFunc;
		}

		// We've set it at least once.
		mStateCache.depthStencil.firstSet = false;
	}

	void GLGraphicsDevice::_cullStateCmd(CullStateCommand *cmd)
	{
		// If this is the first time we are setting the state block,
		// update everything.
		bool first = mStateCache.cull.firstSet;

		// Enables/Disables face culling
		if (first || (cmd->enabled != mStateCache.cull.enabled))
		{
			if (cmd->enabled)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);
			mStateCache.cull.enabled = cmd->enabled;
		}

		// Sets which face to cull
		if (first || (cmd->face != mStateCache.cull.face))
		{
			if (cmd->face == CullFaceState::eBack)
				glCullFace(GL_BACK);
			else
				glCullFace(GL_FRONT);
			mStateCache.cull.face = cmd->face;
		}

		// Winding order
		if (first || (cmd->state != mStateCache.cull.state))
		{
			if (cmd->state == WindingOrderState::eCCW)
				glFrontFace(GL_CCW);
			else
				glFrontFace(GL_CW);
			mStateCache.cull.state = cmd->state;
		}

		// We've set it at least once.
		mStateCache.cull.firstSet = false;
	}

	void GLGraphicsDevice::present()
	{
		glfwSwapBuffers(mWindowHandle);
	}
}