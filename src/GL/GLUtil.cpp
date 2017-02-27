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
		std::unordered_map<TextureWrapUType, GLenum> sUTexCoordToGL;
		std::unordered_map<TextureWrapVType, GLenum> sVTexCoordToGL;
		std::unordered_map<TextureMagnificationType, GLenum> sTexMagnificationToGL;
		std::unordered_map<TextureMinificationType, GLenum> sTexMinificationToGL;
		std::unordered_map<BufferUsageHint, GLenum> sBufferUsageHintToGL;
		std::unordered_map<BufferType, GLenum> sBufferTypeToGL;
		std::unordered_map<VertexAttributeType, GLenum> sVertexAttributeTypeToGL;
		std::unordered_map<ShaderStage, GLenum> sShaderStageToGL;
		std::unordered_map<PrimitiveType, GLenum> sPrimitiveTypeToGL;
		std::unordered_map<BlendState, GLenum> sBlendStateToGL;
		std::unordered_map<DepthFunc, GLenum> sDepthFuncToGL;
		std::unordered_map<TextureInternalFormatType, GLenum> sTextureInternalFormatToGL;
		std::unordered_map<TextureFormatType, GLenum> sTextureFormatToGL;
		std::unordered_map<TextureDataType, GLenum> sTextureDataType;

		void printDeviceInfo()
		{
			std::printf("Vendor: %s\n", glGetString(GL_VENDOR));
			std::printf("Renderer: %s\n", glGetString(GL_RENDERER));
			std::printf("Version: %s\n", glGetString(GL_VERSION));
		}

		void initLookupTables()
		{
			// U texture coordinates
			sUTexCoordToGL[TextureWrapUType::eClampToEdge] = GL_CLAMP_TO_EDGE;
			sUTexCoordToGL[TextureWrapUType::eMirroredRepeat] = GL_MIRRORED_REPEAT;
			sUTexCoordToGL[TextureWrapUType::eRepeat] = GL_REPEAT;
			sUTexCoordToGL[TextureWrapUType::eClampToBoarder] = GL_CLAMP_TO_BORDER;

			// V texture coordinates
			sVTexCoordToGL[TextureWrapVType::eClampToEdge] = GL_CLAMP_TO_EDGE;
			sVTexCoordToGL[TextureWrapVType::eMirroredRepeat] = GL_MIRRORED_REPEAT;
			sVTexCoordToGL[TextureWrapVType::eRepeat] = GL_REPEAT;

			// texture magnification
			sTexMagnificationToGL[TextureMagnificationType::eNearest] = GL_NEAREST;
			sTexMagnificationToGL[TextureMagnificationType::eLinear] = GL_LINEAR;

			// texture minification
			sTexMinificationToGL[TextureMinificationType::eNearest] = GL_NEAREST;
			sTexMinificationToGL[TextureMinificationType::eLinear] = GL_LINEAR;
			sTexMinificationToGL[TextureMinificationType::eNearestMipmapNearest] = GL_NEAREST_MIPMAP_NEAREST;
			sTexMinificationToGL[TextureMinificationType::eLinearMipmapNearest] = GL_LINEAR_MIPMAP_NEAREST;
			sTexMinificationToGL[TextureMinificationType::eNearestMipmapLinear] = GL_NEAREST_MIPMAP_LINEAR;
			sTexMinificationToGL[TextureMinificationType::eLinearMipmapLinear] = GL_LINEAR_MIPMAP_LINEAR;

			// Buffer usage hints
			sBufferUsageHintToGL[BufferUsageHint::eStaticDraw] = GL_STATIC_DRAW;
			sBufferUsageHintToGL[BufferUsageHint::eDynamicDraw] = GL_DYNAMIC_DRAW;
			sBufferUsageHintToGL[BufferUsageHint::eStreamDraw] = GL_STREAM_DRAW;

			// Buffer type
			sBufferTypeToGL[BufferType::eVertexBuffer] = GL_ARRAY_BUFFER;
			sBufferTypeToGL[BufferType::eIndexBuffer] = GL_ELEMENT_ARRAY_BUFFER;
			sBufferTypeToGL[BufferType::eConstantBuffer] = GL_UNIFORM_BUFFER;

			// Vertex Attribute Type
			sVertexAttributeTypeToGL[VertexAttributeType::eFLOAT] = GL_FLOAT;

			// Shader stage
			sShaderStageToGL[ShaderStage::eVertex] = GL_VERTEX_SHADER;
			sShaderStageToGL[ShaderStage::eFragment] = GL_FRAGMENT_SHADER;
			sShaderStageToGL[ShaderStage::eGeometry] = GL_GEOMETRY_SHADER;
			sShaderStageToGL[ShaderStage::eCompute] = GL_COMPUTE_SHADER;

			// Primitive Types
			sPrimitiveTypeToGL[PrimitiveType::eTriangles] = GL_TRIANGLES;
			sPrimitiveTypeToGL[PrimitiveType::eTriangleStrip] = GL_TRIANGLE_STRIP;
			sPrimitiveTypeToGL[PrimitiveType::eLines] = GL_LINES;
			sPrimitiveTypeToGL[PrimitiveType::eLineStrip] = GL_LINE_STRIP;

			// Blend State
			sBlendStateToGL[BlendState::eZero] = GL_ZERO;
			sBlendStateToGL[BlendState::eOne] = GL_ONE;
			sBlendStateToGL[BlendState::eSrcColor] = GL_SRC_COLOR;
			sBlendStateToGL[BlendState::eOneMinusSrcColor] = GL_ONE_MINUS_SRC_COLOR;
			sBlendStateToGL[BlendState::eSrcAlpha] = GL_SRC_ALPHA;
			sBlendStateToGL[BlendState::eOneMinusSrcAlpha] = GL_ONE_MINUS_SRC_ALPHA;
			sBlendStateToGL[BlendState::eDstAlpha] = GL_DST_ALPHA;
			sBlendStateToGL[BlendState::eOneMinusDstAlpha] = GL_ONE_MINUS_DST_ALPHA;
			sBlendStateToGL[BlendState::eDstColor] = GL_DST_COLOR;
			sBlendStateToGL[BlendState::eOneMinusDstColor] = GL_ONE_MINUS_DST_COLOR;

			// Depth Func
			sDepthFuncToGL[DepthFunc::eNever] = GL_NEVER;
			sDepthFuncToGL[DepthFunc::eAlways] = GL_ALWAYS;
			sDepthFuncToGL[DepthFunc::eLess] = GL_LESS;
			sDepthFuncToGL[DepthFunc::eEqual] = GL_EQUAL;
			sDepthFuncToGL[DepthFunc::eNotEqual] = GL_NOTEQUAL;
			sDepthFuncToGL[DepthFunc::eGreater] = GL_GREATER;
			sDepthFuncToGL[DepthFunc::eLessEqual] = GL_LEQUAL;
			sDepthFuncToGL[DepthFunc::eGreaterEqual] = GL_GEQUAL;

			// Texture Internal Format Type
			sTextureInternalFormatToGL[TextureInternalFormatType::eDepth24] = GL_DEPTH_COMPONENT24;
			sTextureInternalFormatToGL[TextureInternalFormatType::eRGB] = GL_RGB;
			sTextureInternalFormatToGL[TextureInternalFormatType::eRGBA] = GL_RGBA;

			// Texture Format Type
			sTextureFormatToGL[TextureFormatType::eDepth] = GL_DEPTH_COMPONENT;
			sTextureFormatToGL[TextureFormatType::eRGB] = GL_RGB;
			sTextureFormatToGL[TextureFormatType::eRGBA] = GL_RGBA;

			// Texture Data Type
			sTextureDataType[TextureDataType::eUnsignedByte] = GL_UNSIGNED_BYTE;
			sTextureDataType[TextureDataType::eByte] = GL_BYTE;
			sTextureDataType[TextureDataType::eUnsignedShort] = GL_UNSIGNED_SHORT;
			sTextureDataType[TextureDataType::eShort] = GL_SHORT;
			sTextureDataType[TextureDataType::eFloat] = GL_FLOAT;
		}
	}
}
