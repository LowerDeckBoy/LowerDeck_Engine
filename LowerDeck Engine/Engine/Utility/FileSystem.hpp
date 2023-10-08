#pragma once
#include <filesystem>

namespace utility
{
	/// <summary>
	/// Fetch given file extension.
	/// </summary>
	/// <param name="Filename"></param>
	/// <returns></returns>
	inline std::string GetFileExtension(const std::string_view& Filename)
	{
		return std::filesystem::path(Filename).extension().string();
	}

	/// <summary>
	/// Fetch absolute path to given file.
	/// </summary>
	/// <param name="Filename"></param>
	/// <returns></returns>
	inline std::string GetFileAbsolutePath(const std::string_view& Filename)
	{
		return absolute(std::filesystem::path(Filename)).string();
	}

	/// <summary>
	/// Fetch relative path to given file.
	/// </summary>
	/// <param name="Filename"></param>
	/// <returns></returns>
	inline std::string GetFileRelativePath(const std::string_view& Filename)
	{
		return std::filesystem::path(Filename).relative_path().string();
	}

	inline std::string GetParentPath(const std::string_view& Filepath)
	{
		return std::filesystem::path(Filepath).parent_path().string();
	}

	/// <summary>
/// 
/// </summary>
	enum class FileExtension : uint8_t
	{
		eJPG = 0x00,
		eJPEG,
		ePNG,
		eDDS,
		eHDR,
		eInvalid
	};

	/// <summary>
	/// Convert <c>FileExtension</c> enum to corresponding string.
	/// </summary>
	/// <param name="Extension"></param>
	/// <returns></returns>
	std::string FileExtToString(FileExtension Extension);
	
	/// <summary>
	/// 
	/// </summary>
	/// <param name="Filepath"></param>
	/// <returns></returns>
	FileExtension FileExtToEnum(const std::string_view& Filepath);
	
	namespace glTF
	{
		/// <summary>
		/// 
		/// </summary>
		inline std::string GetRelativePath(const std::string& Filename)
		{
			return std::filesystem::path(Filename).relative_path().string();
		}
		/// <summary>
		/// 
		/// </summary>
		inline std::string GetAbsolutePath(const std::string& Filename)
		{
			return absolute(std::filesystem::path("../" + Filename)).string();
		}
		/// <summary>
		/// 
		/// </summary>
		inline std::string GetTexturePath(const std::string& Filename, const std::string& TextureName)
		{
			return std::filesystem::path(Filename).relative_path().parent_path().string() + "/" + TextureName;
		}
		/// <summary>
		/// 
		/// </summary>
		inline std::string GetTexAbsolutePath(const std::string& Filename, const std::string& TextureName)
		{
			return absolute(std::filesystem::path(Filename)).parent_path().string() + "\\" + TextureName;
		}
	}

	// TOOD:
	namespace fbx
	{

	}
}
