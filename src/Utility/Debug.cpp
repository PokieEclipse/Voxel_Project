#include "Debug.h"

void Utility::Timer::EndTimer()
{
	ended = true;

	end = std::chrono::high_resolution_clock::now();

	duration = end - start;

	float ms = duration.count() * 1000.0f;
	std::cout << name << " took " << ms << "ms" << std::endl;
}

Utility::Timer::Timer(std::string name) : name(name)
{
	start = std::chrono::high_resolution_clock::now();
}

Utility::Timer::~Timer()
{
	if (ended)
	{
		return;
	}

	end = std::chrono::high_resolution_clock::now();

	duration = end - start;

	float ms = duration.count() * 1000.0f;
	std::cout << name << " took " << ms << "ms" << std::endl;
}
