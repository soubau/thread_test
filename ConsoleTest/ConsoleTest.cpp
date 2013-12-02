// ConsoleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <memory>				// for std::shared_ptr
#include <map>					// for std::map
//#include <typeinfo>			// for typeid
//#include <algorithm>			// for std::for_each

#include <string>				// for std::string
#include <iostream>				// for std::cin, std::cout

#include "..\ThreadTest\BaseLibrary.h"

//

class ConsoleController : public Controller
{
public:
	ConsoleController()
	{
		controlName[START] = "Start";
		controlName[STOP] = "Stop";
		controlName[QUIT] = "Quit";
	}
	virtual void UpdateControls()
	{
		std::for_each(std::begin(controls), std::end(controls), [&](std::pair<int, bool> control){ std::cout << control.first << "/" << controlName[control.first] << ": " << control.second << "\n"; });
		std::cout << "---\n";
	}
	virtual void OnFinish(Work * work)
	{
		std::cout << "(Done)\n";
		UpdateControls();
		if (typeid(*work) == typeid(QuitWork))
		{
			std::cout << "(Press any number and enter to quit)\n";
			SetDone();
		}
	}

	std::map<int, std::string> controlName;
};

//

int _tmain(int argc, _TCHAR* argv[])
{
	static ConsoleController *consoleController;

	consoleController = new ConsoleController();

	consoleController->EnableControl(START, false);
	consoleController->EnableControl(STOP, false);
	consoleController->EnableControl(QUIT, true);
	consoleController->UpdateControls();

	Work *work = new InitWork(consoleController);

	std::cout << "(Init)\n";
	consoleController->QueueWork(std::make_shared<InitWork>(InitWork(consoleController)));

	int command;
	while (true)
	{
		std::cin >> command;
		if (consoleController->done)
		{
			break;
		}
		if (!consoleController->IsEnabled(command))
		{
			std::cout << "(wrong command)\n";
			continue;
		}
		std::cout << "(" << consoleController->controlName[command] << ")\n";
		switch (command)
		{
		case START:
			consoleController->QueueWork(std::make_shared<StartWork>(StartWork(consoleController)));
			break;
		case STOP:
			consoleController->QueueWork(std::make_shared<StopWork>(StopWork(consoleController)));
			break;
		case QUIT:
			consoleController->QueueWork(std::make_shared<QuitWork>(QuitWork(consoleController)));
			break;
		}
	} 

	delete consoleController;

	return 0;
}

