#pragma once
#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <Windows.h>
#include <debugapi.h>
#include <comdef.h>


#ifndef TraceError
// Tracing where HRESULT error occured.
#define TraceError(DebugMessage)										\
	{																	\
		std::wstring message(DebugMessage);								\
		message.append(L"\nFile: " + std::wstring(__FILEW__));			\
		message.append(L"\nFunction: " + std::wstring(__FUNCTIONW__));	\
		message.append(L"\nLine: " + std::to_wstring(__LINE__) + L"\n");\
		::OutputDebugStringW(message.data());							\
		::MessageBoxW(nullptr, message.data(), L"Error", MB_OK);		\
	}
#endif // TraceError

#ifndef ThrowIfFailed
// Helper function to terminate app when mendatory expression fails.
// Relates to HRESULT errors.
#define ThrowIfFailed(hResult, ...)															\
		if (FAILED(hResult))																\
		{																					\
			std::string args{ std::string(__VA_ARGS__) };									\
			std::wstring wstrArgs{ std::wstring(args.begin(), args.end()) };				\
			std::wstring debugMessage{ std::wstring(_com_error(hResult).ErrorMessage()) };	\
			debugMessage.append(L"\n" + wstrArgs);											\
			TraceError(debugMessage);														\
			throw std::runtime_error("");													\
		}
#endif // ThrowIfFailed

#ifndef ASSERT
// TODO:
#define ASSERT(Expr, message) \
	if (!Expr) \

#endif

namespace utility
{
	/// <summary>
	/// Convert given text to wide string.
	/// </summary>
	/// <param name="Text"></param>
	/// <returns></returns>
	inline std::wstring ToWideString(const std::string_view& Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	/// <summary> Debug Win32 API MessageBox pop up </summary>
	inline void ErrorMessage(const std::string_view& Message)
	{
		::MessageBoxA(nullptr, Message.data(), "Error", MB_OK);
	}

	/// <summary> Debug Win32 API MessageBox pop up </summary>
	inline void ErrorMessage(const std::wstring& Message)
	{
		::MessageBoxW(nullptr, Message.data(), L"Error", MB_OK);
	}
}

namespace debug
{
	/// <summary> Print debug message </summary>
	inline void Print(const std::string& Message)
	{
		::OutputDebugStringA(Message.c_str());
	}

	/// <summary> Print debug message </summary>
	inline void Print(const std::wstring& Message)
	{
		::OutputDebugStringW(Message.data());
	}

}
