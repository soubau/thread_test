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
	START2,
	STOP2,
	QUIT
};

class Controller;				// forward declaration for Work

class Work
{
public:
	Work(Controller * controller_ = nullptr) : controller(controller_) {}
	virtual void OnQueued() = 0;
	virtual void Execute() = 0;
	virtual void OnFinish();

	Controller *controller;
};

class QuitWork;					// forward declaration for Controller

class Controller
{
public:
	Controller() : done(false)
	{
		threads.push_back(std::thread(&Controller::WorkThread, this));
		threads.push_back(std::thread(&Controller::WorkThread, this));
	}
	~Controller()
	{
		std::for_each(std::begin(threads), std::end(threads), std::mem_fun_ref(&std::thread::join));
	}
	void QueueWork(std::shared_ptr<Work> work)
	{
		work->OnQueued();

		std::lock_guard<std::mutex> lock(m);
		queue.push(work);
		cv.notify_one();
	}
	bool IsQuitWork(Work * work)
	{
		return typeid(*work) == typeid(QuitWork);
	}
	void SetDone()
	{
		std::lock_guard<std::mutex> lock(m);
		done = true;
		cv.notify_one();
	}
	bool GetDone()
	{
		return done;
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

	std::vector<std::thread> threads;
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
		controller->EnableControl(START2, false);
		controller->EnableControl(STOP2, false);
		controller->EnableControl(QUIT, true);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		if (!controller->GetDone())
		{
			controller->EnableControl(START, true);
			controller->EnableControl(START2, true);
			Work::OnFinish();
		}
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
		if (!controller->GetDone())
		{
			controller->EnableControl(STOP, true);
			Work::OnFinish();
		}
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
		if (!controller->GetDone())
		{
			controller->EnableControl(START, true);
			Work::OnFinish();
		}
	}
};

class Start2Work : public Work
{
public:
	Start2Work(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(START2, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		if (!controller->GetDone())
		{
			controller->EnableControl(STOP2, true);
			Work::OnFinish();
		}
	}
};

class Stop2Work : public Work
{
public:
	Stop2Work(Controller * controller_) : Work(controller_) {}
	virtual void OnQueued()
	{
		controller->EnableControl(STOP2, false);
		controller->UpdateControls();
	}
	virtual void Execute() 
	{ 
		std::this_thread::sleep_for(std::chrono::seconds(1)); 
	}
	virtual void OnFinish()
	{
		if (!controller->GetDone())
		{
			controller->EnableControl(START2, true);
			Work::OnFinish();
		}
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
		controller->EnableControl(START2, false);
		controller->EnableControl(STOP2, false);
		controller->EnableControl(QUIT, false);
		controller->UpdateControls();
		controller->SetDone();
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
