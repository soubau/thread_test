// BaseLibrary.cpp
//

#include "stdafx.h"

#include <thread>				// for std::thread
#include <condition_variable>	// for std::condition_variable
#include <memory>				// for std::shared_ptr
#include <queue>				// for std::queue
#include <map>					// for std::map
//#include <typeinfo>			// for typeid
//#include <mutex>				// for std::mutex
//#include <algorithm>			// for std::for_each
//#include <functional>			// for std::function

//

class Controller;

class Work
{
public:
	Work(Controller * controller_ = nullptr) : controller(controller_) {}
	virtual void OnQueued() {};
	virtual void Execute() {};
	virtual void OnFinish() {}

	Controller *controller;
};

class InitWork : public Work
{
public:
	virtual void Execute() { Sleep(1000 + rand() % 2000); }
};

class StartWork : public Work
{
public:
	virtual void Execute() { Sleep(1000 + rand() % 2000); }
};

class StopWork : public Work
{
public:
	virtual void Execute() { Sleep(1000 + rand() % 2000); }
};

class QuitWork : public Work
{
public:
	virtual void Execute() { Sleep(1000 + rand() % 2000); }
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

	virtual void QueueWork(std::shared_ptr<Work> work)
	{
		std::lock_guard<std::mutex> lock(m);
		queue.push(work);
		cv.notify_one();
	}
	virtual void SetDone()
	{
		std::lock_guard<std::mutex> lock(m);
		done = true;
		cv.notify_one();
	}
	virtual void WorkThread() 
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

	std::thread thread;
	std::mutex m;
	std::condition_variable cv;
	std::queue<std::shared_ptr<Work>> queue;
	bool done;
};
