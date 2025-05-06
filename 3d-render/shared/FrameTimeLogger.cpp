#include "FrameTimeLogger.h"


std::string FrameTimeLogger::GenerateUniqueFileName() const
{
	const std::filesystem::path folder = "log";

	if (!std::filesystem::exists(folder))
	{
		std::filesystem::create_directory(folder);
	}

	int index = 0;
	std::filesystem::path fileName;

	do
	{
		fileName = folder / std::format("frametime_log_{}.txt", ++index);
	} while (std::filesystem::exists(fileName));

	return fileName.string();
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
		try
		{
			logFile.close();
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error closing log file: " << e.what() << std::endl;
		}
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
