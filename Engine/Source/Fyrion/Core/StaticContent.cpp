#include "StaticContent.hpp"

#include "Span.hpp"
#include "cmrc/cmrc.hpp"
#include "Fyrion/Graphics/Graphics.hpp"

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

	Texture GetTextureFile(StringView path)
	{
		Image image = GetImageFile(path);


		Texture texture = Graphics::CreateTexture(TextureCreation{
			.extent = {image.GetWidth(), image.GetHeight(), 1}
		});

		TextureDataRegion region{
			.extent = {image.GetWidth(), image.GetHeight(), 1}
		};

		Graphics::UpdateTextureData(TextureDataInfo{
			.texture = texture,
			.data = image.data.begin(),
			.size = image.data.Size(),
			.regions = {&region, 1}
		});

		return texture;
	}
}
