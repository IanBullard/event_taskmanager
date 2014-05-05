/*
 * Copyright (c) 2014 Ian C. Bullard
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

#include "taskmanager/taskmanager.h"

class EndTask : public Task {
public:
	EndTask(TaskManager* tm) : Task(SINGLETHREADED), tm_(tm) {}
	~EndTask() {}

	void run() final { tm_->stop(); }

	TaskManager* tm_;
};

std::atomic<size_t> completeCount;

class FakeMultithreaded : public Task {
public: 
	FakeMultithreaded(size_t workload) : Task(BACKGROUND), work_(workload) {}
	~FakeMultithreaded() {}

	void run() final {
		// take the workload and do something with it
		result_ = 0;
		for(int i = 0; i < work_; ++i) {
			result_ += (int)((double)work_ / 20.0);
		}
		std::atomic_fetch_add<size_t>(&completeCount, 1);
	}

	size_t work_;
	int result_;
};

class CheckedStop : public Task {
public:
	CheckedStop(TaskManager* tm) : Task(SINGLETHREADED_REPEATING), tm_(tm) {}
	~CheckedStop() {}

	void run() final {
		if(std::atomic_load(&completeCount) == 20) {
			tm_->stop();
		}
	}

	TaskManager* tm_;
};

int main(int argc, char** argv) {
	TaskManager tm;

	// try queue with no multithreaded tasks
	tm.add(TaskManager::TaskPtr(new EndTask(&tm)));
	tm.start();

	// try a bunch of background tasks and one that waits for everything to be complete
	std::atomic_store<size_t>(&completeCount, 0);
	completeCount = 0;
	for(int i = 0; i < 4; ++i) {
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(65536)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(4096)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(10000000)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(2)));
		tm.add(TaskManager::TaskPtr(new FakeMultithreaded(1024*1024*1024)));
	}
	tm.add(TaskManager::TaskPtr(new CheckedStop(&tm)));

	tm.start();
}