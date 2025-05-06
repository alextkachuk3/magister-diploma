#pragma once

#include <fstream>
#include <filesystem>
#include <format>
#include <string>
#include <iostream>

class FrameTimeLogger
{
private:
	std::ofstream logFile;

	std::string GenerateUniqueFileName() const;

public:
	FrameTimeLogger();
	~FrameTimeLogger();

	void LogFrameTime(float frameTime);
};
