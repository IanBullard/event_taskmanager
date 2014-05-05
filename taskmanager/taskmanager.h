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
#include <thread>
#include <algorithm>
#include "concurrent_queue.h"
#include "task.h"
#include "event/eventchannel.h"

// Note: when using this there must be at least one background task!
class TaskManager {
public:
	typedef std::shared_ptr<Task> TaskPtr;
	typedef ConcurrentQueue<TaskPtr> TaskList;
	
	struct StopEvent {};
	
	TaskManager(unsigned int numThreads = 0); //0 for autodetect
	~TaskManager();
	
	void add(TaskPtr task);
	
	void start();
	void stop();
	
	void handle(const StopEvent& );
	void handle(const Task::TaskCompleted& tc);
	
private:
	void worker();
	void execute(TaskPtr task);
	void synchronize();
	
  std::list<std::thread*> mThreads;
	unsigned int mNumThreads;
	
	bool mRunning;
	
	TaskList mTaskList[2];
	TaskList mBackgroundTasks;
	TaskList mSyncTasks;
	
	unsigned int mReadList;
	unsigned int mWriteList;
	
	typedef std::mutex Mutex;
	typedef std::condition_variable Condition;
	typedef std::lock_guard<Mutex> ScopedLock;
	
	mutable Mutex mSyncMutex;
	Condition mCondition;
  size_t mNumTasksToWaitFor;
};
