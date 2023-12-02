#include "FileSystem.hpp"


namespace utility
{
	std::string FileExtToString(FileExtension Extension)
	{
		switch (Extension)
		{
		case utility::FileExtension::eJPG:
			return ".jpg";
		case utility::FileExtension::eJPEG:
			return ".jpeg";
		case utility::FileExtension::ePNG:
			return ".png";
		case utility::FileExtension::eDDS:
			return ".dds";
		case utility::FileExtension::eHDR:
			return ".hdr";
		case utility::FileExtension::eInvalid:
		default:
			return "Invalid";
		}
	}

	FileExtension FileExtToEnum(const std::string_view& Filepath)
	{
		auto extension{ GetFileExtension(Filepath) };

		if (extension == ".jpg")
			return FileExtension::eJPG;
		else if (extension == ".jpeg")
			return FileExtension::eJPEG;
		else if (extension == ".png")
			return FileExtension::ePNG;
		else if (extension == ".dds")
			return FileExtension::eDDS;
		else if (extension == ".hdr")
			return FileExtension::eHDR;
		else
			return FileExtension::eInvalid;
	}
}
