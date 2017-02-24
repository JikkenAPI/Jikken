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

#include "shaderUtils.hpp"
#include <SPIRV/GlslangToSpv.h>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>

namespace Jikken
{
	namespace ShaderUtils
	{
		//todo lookup table be faster
		EShLanguage _findLanguage(const ShaderStage &stage)
		{
			switch (stage)
			{
			case ShaderStage::eCompute:
				return EShLangCompute;

			case ShaderStage::eFragment:
				return EShLangFragment;

			case ShaderStage::eGeometry:
				return EShLangGeometry;

			case ShaderStage::eVertex:
				return EShLangVertex;

			default:
				return EShLangVertex;
			}
		}

		// the struct is large enough to sink the titanic - todo might need to make this match our lowest common dominator
		void _initResources(TBuiltInResource &Resources)
		{
			Resources.maxLights = 32;
			Resources.maxClipPlanes = 6;
			Resources.maxTextureUnits = 32;
			Resources.maxTextureCoords = 32;
			Resources.maxVertexAttribs = 64;
			Resources.maxVertexUniformComponents = 4096;
			Resources.maxVaryingFloats = 64;
			Resources.maxVertexTextureImageUnits = 32;
			Resources.maxCombinedTextureImageUnits = 80;
			Resources.maxTextureImageUnits = 32;
			Resources.maxFragmentUniformComponents = 4096;
			Resources.maxDrawBuffers = 32;
			Resources.maxVertexUniformVectors = 128;
			Resources.maxVaryingVectors = 8;
			Resources.maxFragmentUniformVectors = 16;
			Resources.maxVertexOutputVectors = 16;
			Resources.maxFragmentInputVectors = 15;
			Resources.minProgramTexelOffset = -8;
			Resources.maxProgramTexelOffset = 7;
			Resources.maxClipDistances = 8;
			Resources.maxComputeWorkGroupCountX = 65535;
			Resources.maxComputeWorkGroupCountY = 65535;
			Resources.maxComputeWorkGroupCountZ = 65535;
			Resources.maxComputeWorkGroupSizeX = 1024;
			Resources.maxComputeWorkGroupSizeY = 1024;
			Resources.maxComputeWorkGroupSizeZ = 64;
			Resources.maxComputeUniformComponents = 1024;
			Resources.maxComputeTextureImageUnits = 16;
			Resources.maxComputeImageUniforms = 8;
			Resources.maxComputeAtomicCounters = 8;
			Resources.maxComputeAtomicCounterBuffers = 1;
			Resources.maxVaryingComponents = 60;
			Resources.maxVertexOutputComponents = 64;
			Resources.maxGeometryInputComponents = 64;
			Resources.maxGeometryOutputComponents = 128;
			Resources.maxFragmentInputComponents = 128;
			Resources.maxImageUnits = 8;
			Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
			Resources.maxCombinedShaderOutputResources = 8;
			Resources.maxImageSamples = 0;
			Resources.maxVertexImageUniforms = 0;
			Resources.maxTessControlImageUniforms = 0;
			Resources.maxTessEvaluationImageUniforms = 0;
			Resources.maxGeometryImageUniforms = 0;
			Resources.maxFragmentImageUniforms = 8;
			Resources.maxCombinedImageUniforms = 8;
			Resources.maxGeometryTextureImageUnits = 16;
			Resources.maxGeometryOutputVertices = 256;
			Resources.maxGeometryTotalOutputComponents = 1024;
			Resources.maxGeometryUniformComponents = 1024;
			Resources.maxGeometryVaryingComponents = 64;
			Resources.maxTessControlInputComponents = 128;
			Resources.maxTessControlOutputComponents = 128;
			Resources.maxTessControlTextureImageUnits = 16;
			Resources.maxTessControlUniformComponents = 1024;
			Resources.maxTessControlTotalOutputComponents = 4096;
			Resources.maxTessEvaluationInputComponents = 128;
			Resources.maxTessEvaluationOutputComponents = 128;
			Resources.maxTessEvaluationTextureImageUnits = 16;
			Resources.maxTessEvaluationUniformComponents = 1024;
			Resources.maxTessPatchComponents = 120;
			Resources.maxPatchVertices = 32;
			Resources.maxTessGenLevel = 64;
			Resources.maxViewports = 16;
			Resources.maxVertexAtomicCounters = 0;
			Resources.maxTessControlAtomicCounters = 0;
			Resources.maxTessEvaluationAtomicCounters = 0;
			Resources.maxGeometryAtomicCounters = 0;
			Resources.maxFragmentAtomicCounters = 8;
			Resources.maxCombinedAtomicCounters = 8;
			Resources.maxAtomicCounterBindings = 1;
			Resources.maxVertexAtomicCounterBuffers = 0;
			Resources.maxTessControlAtomicCounterBuffers = 0;
			Resources.maxTessEvaluationAtomicCounterBuffers = 0;
			Resources.maxGeometryAtomicCounterBuffers = 0;
			Resources.maxFragmentAtomicCounterBuffers = 1;
			Resources.maxCombinedAtomicCounterBuffers = 1;
			Resources.maxAtomicCounterBufferSize = 16384;
			Resources.maxTransformFeedbackBuffers = 4;
			Resources.maxTransformFeedbackInterleavedComponents = 64;
			Resources.maxCullDistances = 8;
			Resources.maxCombinedClipAndCullDistances = 8;
			Resources.maxSamples = 4;
			Resources.limits.nonInductiveForLoops = 1;
			Resources.limits.whileLoops = 1;
			Resources.limits.doWhileLoops = 1;
			Resources.limits.generalUniformIndexing = 1;
			Resources.limits.generalAttributeMatrixVectorIndexing = 1;
			Resources.limits.generalVaryingIndexing = 1;
			Resources.limits.generalSamplerIndexing = 1;
			Resources.limits.generalVariableIndexing = 1;
			Resources.limits.generalConstantMatrixVectorIndexing = 1;
		}

		bool validateGlsl(const ShaderStage &stage, const std::string &glslSrc)
		{
			EShLanguage type = _findLanguage(stage);
			glslang::TShader shader(type);
			glslang::TProgram program;
			TBuiltInResource Resources;
			_initResources(Resources);

			EShMessages messages = (EShMessages)(EShMsgSpvRules);

			const char *shaderSrc[1];
			shaderSrc[0] = glslSrc.c_str();
			shader.setStrings(shaderSrc, 1);

			if (!shader.parse(&Resources, 330, false, messages))
			{
				std::printf("Shader parse error: Info log %s", shader.getInfoLog());
				std::printf("Shader parse error: Info Debug log %s", shader.getInfoDebugLog());
				return false;
			}

			program.addShader(&shader);

			if (!program.link(messages))
			{
				std::printf("Shader link error: Info log %s", shader.getInfoLog());
				std::printf("Shader link error: Info Debug log %s", shader.getInfoDebugLog());
				return false;
			}

			return true;
		}

		bool convertGlslToSpirv(const ShaderStage &stage, const std::string &glslIn, std::vector<uint32_t> &spirv)
		{
			EShLanguage type = _findLanguage(stage);
			glslang::TShader shader(type);
			glslang::TProgram program;
			TBuiltInResource Resources;
			_initResources(Resources);

			EShMessages messages = (EShMessages)(EShMsgSpvRules);

			const char *shaderSrc[1];
			shaderSrc[0] = glslIn.c_str();
			shader.setStrings(shaderSrc, 1);

			if (!shader.parse(&Resources, 330, false, messages))
			{
				std::printf("Shader parse error: Info log %s", shader.getInfoLog());
				std::printf("Shader parse error: Info Debug log %s", shader.getInfoDebugLog());
				return false;
			}

			program.addShader(&shader);

			if (!program.link(messages))
			{
				std::printf("Shader link error: Info log %s", shader.getInfoLog());
				std::printf("Shader link error: Info Debug log %s", shader.getInfoDebugLog());
				return false;
			}

			glslang::GlslangToSpv(*program.getIntermediate(type), spirv);


			return true;
		}

		bool convertSpirvToGlsl(const ShaderStage &stage, const std::vector<uint32_t> &spirvIn, std::string &glslOut)
		{
			try
			{
				spirv_cross::CompilerGLSL glsl(spirvIn);
				spirv_cross::CompilerGLSL::Options options;
				options.version = 330;
				options.es = false;
				glsl.set_options(options);

				glslOut = glsl.compile();
			}
			catch(spirv_cross::CompilerError err)
			{
				std::printf("Spir-v Cross error: %s",err.what());
				return false;
			}

			return true;
			
		}

		bool convertSpirvToHlsl(const ShaderStage &stage, const std::vector<uint32_t> &spirvIn, std::string &hlslOut)
		{
			try
			{
				spirv_cross::CompilerHLSL hlsl(spirvIn);
				spirv_cross::CompilerHLSL::Options options;
				//options.flip_vert_y = true;
				//options.fixup_clipspace = true;
				options.shader_model = 50;
				hlsl.set_options(options);

				hlslOut = hlsl.compile();
			}
			catch (spirv_cross::CompilerError err)
			{
				std::printf("Spir-v Cross error: %s", err.what());
				return false;
			}

			return true;
		}

		bool convertSpirvToMsl(const ShaderStage &stage, const std::vector<uint32_t> &spirvIn, std::string &mslOut)
		{
			try
			{
				spirv_cross::CompilerMSL msl(spirvIn);
				spirv_cross::CompilerMSL::Options options;
				msl.set_options(options);

				mslOut = msl.compile();
			}
			catch (spirv_cross::CompilerError err)
			{
				std::printf("Spir-v Cross error: %s", err.what());
				return false;
			}

			return true;
		}
	}
}