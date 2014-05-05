/*
 * Copyright (c) 2014 GrandMaster (gijsber@gmail)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <memory>

class TaskManager;

class Task {
public:
	typedef std::shared_ptr<Task> TaskPtr;

	struct TaskBeginning {
		TaskBeginning(TaskPtr t);
		TaskPtr mTask;
	};
	
	struct TaskCompleted {
		TaskCompleted(TaskPtr t);
		TaskPtr mTask;
	};
	
	enum {
		NONE = 0x0,
		
		REPEATING = 0x1 << 0,
		THREADSAFE = 0x1 << 1,
		FRAME_SYNC = 0x1 << 2,
		
		SINGLETHREADED = NONE,
		SINGLETHREADED_REPEATING = REPEATING,	
		BACKGROUND = THREADSAFE,
		BACKGROUND_REPEATING = THREADSAFE | REPEATING,
		BACKGROUND_SYNC = THREADSAFE | FRAME_SYNC,
		BACKGROUND_SYNC_REPEATING = THREADSAFE | REPEATING | FRAME_SYNC,
		
		ALL = ~0x0
	};
	
	Task(unsigned int flags = SINGLETHREADED_REPEATING);
	virtual ~Task();
	
	virtual void run() = 0;
  
  void stopRepeating() { mTaskFlags &= ~REPEATING; }
	
protected:
	friend class TaskManager;
	
	unsigned int getTaskFlags() const;
	
private:
	unsigned int mTaskFlags;
};
