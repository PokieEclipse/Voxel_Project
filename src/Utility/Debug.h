#pragma once
#include <iostream>
#include <chrono>
#include <thread>

namespace Utility {

	// Says how much time a function has taken in milliseconds
	struct Timer {

		std::chrono::time_point<std::chrono::steady_clock> start, end;
		std::chrono::duration<float> duration;
		bool ended = false;

		void EndTimer();

		std::string name;
		Timer(std::string name);
		~Timer();
	};

}
