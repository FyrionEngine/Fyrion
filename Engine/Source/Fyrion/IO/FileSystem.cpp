
#include "FileSystem.hpp"
#include "Path.hpp"

//TODO remove filesystem and implement it native files.
#include <filesystem>
namespace fs = std::filesystem;

namespace Fyrion
{
    namespace
    {
        char homeDir[512] = {};
    }

    String FileSystem::AssetFolder()
    {
        String currentDir = CurrentDir();
        String assetDir = Path::Join(currentDir, "Assets");
        while (!GetFileStatus(assetDir).exists)
        {
            currentDir = Path::Parent(currentDir);
            if (!GetFileStatus(currentDir).exists)
            {
                return CurrentDir();
            }
            assetDir = Path::Join(currentDir, "Assets");
        }
        return assetDir;
    }

    String FileSystem::TempFolder()
    {
        return {};
    }

    bool FileSystem::CreateDirectory(const StringView& path)
    {
        return fs::create_directories(fs::path(path.begin(), path.end()));
    }

    bool FileSystem::Remove(const StringView& path)
    {
        return fs::remove_all(path.CStr());
    }

    bool FileSystem::Rename(const StringView& newName, const StringView& oldName)
    {
        std::error_code ec {};
        fs::rename(newName.CStr(), oldName.CStr(), ec);
        return ec.value() == 0;
    }

    bool FileSystem::CopyFile(const StringView& from, const StringView& to)
    {
        auto toPath = fs::path(to.begin(), to.end());
        const auto copyOptions = fs::copy_options::update_existing | fs::copy_options::recursive;
        std::error_code ec{};
        fs::copy(fs::path(from.begin(), from.end()), toPath, copyOptions, ec);
        return ec.value() == 0;
    }

    String FileSystem::ReadFileAsString(const StringView& path)
    {
        String ret{};
        FileHandler fileHandler = OpenFile(path, AccessMode::ReadOnly);
        ret.Resize(GetFileSize(fileHandler));
        ReadFile(fileHandler, ret.begin(), ret.Size());
        return ret;
    }

    Array<u8> FileSystem::ReadFileAsByteArray(const StringView& path)
    {
        Array<u8> ret{};
        FileHandler fileHandler = OpenFile(path, AccessMode::ReadOnly);
        ret.Resize(GetFileSize(fileHandler));
        ReadFile(fileHandler, ret.begin(), ret.Size());
        return ret;
    }
}