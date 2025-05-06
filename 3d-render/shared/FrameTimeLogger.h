#pragma once

#include <fstream>
#include <filesystem>
#include <format>
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
