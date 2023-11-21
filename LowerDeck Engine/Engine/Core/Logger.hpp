#pragma once
#include <ImGui/imgui.h>
#include <string>
#include <vector>

class Logger
{
public:
	static void PrintAll()
	{
		for (const auto& log : Logs)
		{
			ImGui::Text(log.c_str());
		}
	}

	static void ClearLogs()
	{
		if (ImGui::Button("Clear logs"))
		{
			Logs.clear();
		}
	}

	static std::vector<std::string> Logs;

};

#define LOG_INFO(Message)		Logger::Logs.push_back(std::string("[Info] " + std::string(Message)))
#define LOG_WARN(Message)		Logger::Logs.push_back(std::string("[Warn] " + std::string(Message)))
#define LOG_ERROR(Message)		Logger::Logs.push_back(std::string("[Error] " + std::string(Message)))
#define LOG_CRITICAL(Message)	Logger::Logs.push_back(std::string("[Critical] " + std::string(Message)))

#if defined _DEBUG
#define LOG_DEBUG(DebugMessage) Logger::Logs.push_back(std::string("[Debug] " + std::string(DebugMessage)))
#endif

