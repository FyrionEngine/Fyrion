#include "StaticContent.hpp"

#include "Span.hpp"
#include "cmrc/cmrc.hpp"

CMRC_DECLARE(StaticContent);


namespace Fyrion::StaticContent
{
	Array<u8> GetBinaryFile(StringView path)
	{
		auto fs = cmrc::StaticContent::get_filesystem();
		auto file = fs.open(path.CStr());
		return {file.begin(), file.end()};
	}

	String GetTextFile(StringView path)
	{
		auto fs = cmrc::StaticContent::get_filesystem();
		auto file = fs.open(path.CStr());

		//workaround to remove \r from the strings
		std::string str((const char*)file.begin(), file.size());
		str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
		return {str.c_str(), str.size()};
	}

	Image GetImageFile(StringView path)
	{
		Array<u8> bytes =GetBinaryFile(path);
		return {Span<const u8>(bytes.begin(), bytes.end())};
	}
}