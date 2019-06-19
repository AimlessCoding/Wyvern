#include "WyvWindow.h"

#if defined _WIN32
#include <Windows.h>
#endif //WINDOWS

#include <iostream>

#define WINDOW_HEIGHT 1080
#define ASPECT_RATIO 16.0f / 9.0f
#define WINDOW_WIDTH WINDOW_HEIGHT * ASPECT_RATIO

void LogCallback(wyv::WyvCode _code, std::string _message);

int main()
{
	try
	{
		wyv::Wyvern::SetMessageCallback(&LogCallback);
#ifndef NDEBUG
		wyv::Wyvern::SetLogVerbosity(wyv::WYV_MESSAGE);
#endif //NDEBUG
		wyv::Wyvern::Initialize();
		{
			wyv::SharedWindow window = wyv::WyvWindow::CreateShared("Vulkan Wyvern", WINDOW_WIDTH, WINDOW_HEIGHT);

			while (!window->shouldClose())
				wyv::Wyvern::PollEvents();
		}
		wyv::Wyvern::Terminate();
	}
	catch (std::runtime_error _e)
	{
		std::cerr << "Runtime error detected: " << _e.what() << std::endl;
	}
	catch (std::exception _e)
	{
		std::cerr << "Exception thrown: " << _e.what() << std::endl;
#ifdef _WIN32
		system("pause");
#endif //_WIN32
		return 0;
	}
	return 0;
}

void LogCallback(wyv::WyvCode _code, std::string _message)
{
#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if(_code >= wyv::WYV_FAILURE)
		SetConsoleTextAttribute(hConsole, BACKGROUND_RED);
	else if (_code >= wyv::WYV_ERROR)
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
	else if(_code >= wyv::WYV_WARNING)
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	else if(_code >= wyv::WYV_MESSAGE)
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN);

	std::cout << _message << std::endl;

	SetConsoleTextAttribute(hConsole, 0);
#elif defined unix || defined __unix || defined __unix__
	std::string colorCode;
	if (_code >= wyv::WYV_FAILURE)
		colorCode = "40";
	else if (_code == wyv::WYV_ERROR)
		colorCode == "31";
	else if (_code == wyv::WYV_WARNING)
		colorCode == "33";
	else if (_code == wyv::WYV_MESSAGE)
		colorCode == "36";

	std::cout << "\033[0;" << colorCode << "m" << _message << "\033[0m" << std::endl;
#endif //WINDOWS VS UNIX
}