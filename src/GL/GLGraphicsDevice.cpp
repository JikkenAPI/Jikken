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
#include "GLGraphicsDevice.hpp"
#include "GLUtil.hpp"

namespace Jikken
{
	GLGraphicsDevice::GLGraphicsDevice()
	{
		mBufferHandle = 0;
		mVertexArrayHandle = 0;
		mShaderHandle = 0;
		mLayoutHandle = 0;

		glGenVertexArrays(1, &mGlobalVAO);
		glBindVertexArray(mGlobalVAO);
	}

	GLGraphicsDevice::~GLGraphicsDevice()
	{
		glDeleteVertexArrays(1, &mGlobalVAO);
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
				glGetShaderInfoLog(attachment, LENGTH, &info);
				printf("Shader %s compile error.\n", details.file.c_str());
				printf(info);
				abort();
			}
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

		mBufferToGL[handle] = { type, buffer };
		return handle;
	}

	LayoutHandle GLGraphicsDevice::createVertexInputLayout(const std::vector<VertexInputLayout> &attributes)
	{
		// Since the VAO handles layout in GL, we just copy it and store it.
		LayoutHandle layout = mLayoutHandle++;
		for (const VertexInputLayout &attr : attributes)
			mLayoutToGL[layout].push_back(attr);
		return layout;
	}

	VertexArrayHandle GLGraphicsDevice::createVAO(LayoutHandle layout, BufferHandle vertexBuffer, BufferHandle indexBuffer = 0)
	{
#ifdef _DEBUG
		if (mBufferToGL[vertexBuffer].type != BufferType::eVertexBuffer)
			assert(false);
		if (indexBuffer != 0 && mBufferToGL[indexBuffer].type != BufferType::eIndexBuffer)
			assert(false);
#endif
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		{
			// Bind Vertex Buffer
			glBindBuffer(GL_ARRAY_BUFFER, mBufferToGL[vertexBuffer].buffer);

			// Index buffer is optional. We can draw without index buffers in OpenGL.
			if (indexBuffer > 0)
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
		
		VertexArrayHandle handle = mVertexArrayHandle++;
		mVertexArrayToGL[handle] = { vertexBuffer, indexBuffer, layout, vao };
		return handle;
	}

	void GLGraphicsDevice::bindConstantBuffer(ShaderHandle shader, BufferHandle cBuffer, int32_t index)
	{
#ifdef _DEBUG
		if (mBufferToGL[cBuffer].type != BufferType::eConstantBuffer)
			assert(false);
#endif
		// Bind the uniform block to the shader program at index
		glUniformBlockBinding(mShaderToGL[shader].program, index, mBufferToGL[cBuffer].buffer);
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

	void GLGraphicsDevice::submitCommandQueue()
	{
		for (CommandQueue *cmdQueue : mCommandQueuePool)
		{
			// Process all commands and then clear the queue so it can be filled again.
			for (ICommand *cmd = cmdQueue->beginList(); cmd != nullptr; cmd = cmd->next)
				_processCmd(cmd);
			cmdQueue->resetQueue();
		}
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

	}

	void GLGraphicsDevice::_updateBufferCmd(UpdateBufferCommand *cmd)
	{

	}

	void GLGraphicsDevice::_drawCmd(DrawCommand *cmd)
	{

	}

	void GLGraphicsDevice::_drawInstanceCmd(DrawInstanceCommand *cmd)
	{

	}

	void GLGraphicsDevice::_clearBufferCmd(ClearBufferCommand *cmd)
	{

	}

	void GLGraphicsDevice::_bindVAOCmd(BindVAOCommand *cmd)
	{

	}

	void GLGraphicsDevice::_viewportCmd(ViewportCommand *cmd)
	{

	}

	void GLGraphicsDevice::_blendStateCmd(BlendStateCommand *cmd)
	{

	}

	void GLGraphicsDevice::_depthStencilStateCmd(DepthStencilStateCommand *cmd)
	{

	}

	void GLGraphicsDevice::_cullStateCmd(CullStateCommand *cmd)
	{

	}
}