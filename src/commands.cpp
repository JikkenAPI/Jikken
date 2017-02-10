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

#include "jikken/commands.hpp"

namespace Jikken
{
	ICommand::ICommand()
	{
		commandType = CommandType::eAbstract;
	}

	SetShaderCommand::SetShaderCommand()
	{
		commandType = CommandType::eSetShader;
	}

	UpdateBufferCommand::UpdateBufferCommand()
	{
		commandType = CommandType::eUpdateBuffer;
	}

	DrawCommand::DrawCommand()
	{
		commandType = CommandType::eDraw;
	}

	DrawInstanceCommand::DrawInstanceCommand()
	{
		commandType = CommandType::eDrawInstance;
	}

	ClearBufferCommand::ClearBufferCommand()
	{
		commandType = CommandType::eClearBuffer;
	}

	BindVAOCommand::BindVAOCommand()
	{
		commandType = CommandType::eBindVAO;
	}

	ViewportCommand::ViewportCommand()
	{
		commandType = CommandType::eViewport;
	}

	BlendStateCommand::BlendStateCommand()
	{
		commandType = CommandType::eBlendState;
	}

	DepthStencilStateCommand::DepthStencilStateCommand()
	{
		commandType = CommandType::eDepthStencilState;
	}

	CullStateCommand::CullStateCommand()
	{
		commandType = CommandType::eCullState;
	}
}