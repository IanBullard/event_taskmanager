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

#include "taskmanager.h"

TaskManager::TaskManager(unsigned int numThreads) {
	mNumThreads = numThreads;
	if (numThreads == 0)
		mNumThreads = std::thread::hardware_concurrency() + 1;

	mWriteList = 0;
	mReadList = 1;

  mNumTasksToWaitFor = 0;
}

TaskManager::~TaskManager() {
  for(auto itr : mThreads)
    itr->join();
}

void TaskManager::add(TaskPtr task) {
	unsigned flags = task->getTaskFlags();

	if (flags & Task::THREADSAFE) {
		if (flags & Task::FRAME_SYNC)
			mSyncTasks.push(task);
		else
			mBackgroundTasks.push(task);
	}
	else
		mTaskList[mWriteList].push(task);
}

void TaskManager::start() {
	mRunning = true;

	EventChannel chan;

	chan.add<Task::TaskCompleted>(*this);
	chan.add<StopEvent>(*this);

	for (unsigned int i = 0; i < mNumThreads; ++i)
		mThreads.push_back(new std::thread(std::bind(&TaskManager::worker, this)));

	while (mRunning) {
		if (!mTaskList[mReadList].empty()) {
			TaskPtr t = mTaskList[mReadList].wait_pop();
			execute(t);
		}
		else {
			synchronize();
			std::swap(mReadList, mWriteList);				
		}

		std::this_thread::yield();
	}
}
	
void TaskManager::synchronize() {
  std::unique_lock<std::mutex> lock(mSyncMutex);

	while (mNumTasksToWaitFor > 0)
		mCondition.wait(lock);

  mNumTasksToWaitFor = mSyncTasks.size();

	while (!mSyncTasks.empty())
		mBackgroundTasks.push(mSyncTasks.wait_pop());
}

void TaskManager::stop() {		
	mRunning = false;
}

void TaskManager::execute(TaskPtr t) {
	EventChannel chan;
		
	chan.broadcast(Task::TaskBeginning(t));
	t->run();
	chan.broadcast(Task::TaskCompleted(t));
}

void TaskManager::handle(const TaskManager::StopEvent& ) {		
	stop();
}

void TaskManager::handle(const Task::TaskCompleted& tc) {
	if (tc.mTask->getTaskFlags() & Task::REPEATING)
		add(tc.mTask);
}

void TaskManager::worker() {
	TaskPtr task;

	while (mRunning) {
	    bool exec = mBackgroundTasks.try_pop(task);

	    if(exec) {
	      execute(task);

	      if (task->getTaskFlags() & Task::FRAME_SYNC) {
	        {
	          std::lock_guard<std::mutex> lock(mSyncMutex);
	          mNumTasksToWaitFor -= 1;
	        }
	        
	        mCondition.notify_one();
	      }			

	      std::this_thread::yield();
	    } else {
	      // nothing is ready to run, sleep for 1.667 milliseconds (1/10th of a frame @ 60 FPS)
	      std::this_thread::sleep_for(std::chrono::microseconds(1667));
	    }
	}
}