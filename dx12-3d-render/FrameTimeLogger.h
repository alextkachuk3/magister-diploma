#pragma once

#include <fstream>
#include <string>

class FrameTimeLogger
{
private:
	std::ofstream logFile;

	std::string GenerateUniqueFileName();

public:
	FrameTimeLogger();
	~FrameTimeLogger();

	void LogFrameTime(float frameTime);
};
