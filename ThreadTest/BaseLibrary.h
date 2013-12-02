// BaseLibrary.h
//

#pragma once

#include <thread>				// for std::thread
#include <condition_variable>	// for std::condition_variable
#include <memory>				// for std::shared_ptr
#include <queue>				// for std::queue
//#include <mutex>				// for std::mutex
//#include <chrono>				// for std::chrono

//

enum Control
{
	START,
	STOP,
	QUIT
};

class Controller;

class Work
{
public:
	Work(Controller * controller_ = nullptr) : controller(controller_) {}
	virtual void OnQueued() = 0;
	virtual void Execute() = 0;
	virtual void OnFinish();

	Controller *controller;
};

class Controller
{
public:
	Controller() : done(false)
	{
		thread = std::thread(&Controller::WorkThread, this);
	}
	~Controller()
	{
		thread.join();
	}
	void QueueWork(std::shared_ptr<Work> work)
	{
		work->OnQueued();

		std::lock_guard<std::mutex> lock(m);
		queue.push(work);
		cv.notify_one();
	}
	void SetDone()
	{
		std::lock_guard<std::mutex> lock(m);
		done = true;
		cv.notify_one();
	}
	void WorkThread() 
	{
		while (true)
		{
			std::shared_ptr<Work> work;
			{
				std::unique_lock<std::mutex> lock(m);
				cv.wait(lock, [&]{ return !queue.empty() || done; });
				if (queue.empty() && done)
				{
					break;
				}
				work = queue.front();
				queue.pop();
			}
			work->Execute();
			work->OnFinish();
		}
	}
	void EnableControl(int id, bool enabled) 
	{
		controls[id] = enabled;
	}
	bool IsEnabled(int id)
	{
		return controls[id];
	}
	virtual void UpdateControls() {}
	virtual void OnFinish(Work * work) {}

	std::thread thread;
	std::mutex m;
	std::condition_variable cv;
	std::queue<std::shared_ptr<Work>> queue;
	bool done;

	std::map<int, bool> controls;
};

void Work::OnFinish()
{
	controller->OnFinish(this);
}

class InitWork : public Work
{
public:
	InitWork(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(START, false);
		controller->EnableControl(STOP, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		controller->EnableControl(START, true);
		Work::OnFinish();
	}
};

class StartWork : public Work
{
public:
	StartWork(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(START, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		controller->EnableControl(STOP, true);
		Work::OnFinish();
	}
};

class StopWork : public Work
{
public:
	StopWork(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(STOP, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		controller->EnableControl(START, true);
		Work::OnFinish();
	}
};

class QuitWork : public Work
{
public:
	QuitWork(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(START, false);
		controller->EnableControl(STOP, false);
		controller->EnableControl(QUIT, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		Work::OnFinish();
	}
};
