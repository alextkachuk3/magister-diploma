#include "FrameTimeLogger.h"
#include <filesystem>
#include <format>

std::string FrameTimeLogger::GenerateUniqueFileName()
{
	const std::string folder = "log";

	if (!std::filesystem::exists(folder))
	{
		std::filesystem::create_directory(folder);
	}

	int index = 0;
	std::string fileName;

	do
	{
		fileName = std::format("{}/frametime_log_{}.txt", folder, index++);
	} while (std::filesystem::exists(fileName));

	return fileName;
}

FrameTimeLogger::FrameTimeLogger()
{
	std::string fileName = GenerateUniqueFileName();
	logFile.open(fileName, std::ios::out);
}

FrameTimeLogger::~FrameTimeLogger()
{
	if (logFile.is_open())
	{
		logFile.close();
	}
}

void FrameTimeLogger::LogFrameTime(float frameTime)
{
	if (logFile.is_open())
	{
		logFile << frameTime << '\n';
		logFile.flush();
	}
}
