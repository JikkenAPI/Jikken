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

#include "jikken/graphicsDevice.hpp"

namespace Jikken
{
	GraphicsDevice::~GraphicsDevice()
	{
		// Cleanup all command queues.
		for (CommandQueue *queue : mCommandQueuePool)
		{
			delete queue;
			queue = nullptr;
		}
	}

	CommandQueue* GraphicsDevice::createCommandQueue()
	{
		CommandQueue *queue = new CommandQueue();
		mCommandQueuePool.push_back(queue);
		return queue;
	}

	void GraphicsDevice::deleteCommandQueue(CommandQueue *cmdQueue)
	{
		auto pos = std::find(mCommandQueuePool.begin(), mCommandQueuePool.end(), cmdQueue);
		mCommandQueuePool.erase(pos);
		delete cmdQueue;
		cmdQueue = nullptr;
	}
}