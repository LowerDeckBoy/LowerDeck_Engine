#pragma once
#include <ImGui/imgui.h>


/// <summary>
/// Logger class meant for ImGui output usage.
/// </summary>
class Logger
{
public:
	static void Log(const char* Message);

private:
	static ImGuiTextBuffer m_Buffer;
	static ImGuiTextFilter m_Filter;
	static ImVector<int>   m_LineOffsets; // Index to lines offset
	static bool            m_ScrollToBottom;
};

