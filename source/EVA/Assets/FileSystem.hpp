#pragma once

namespace EVA
{
    using Path = std::filesystem::path;

    class FileSystem
    {
        inline static std::filesystem::path s_RootPath = "./assets/";

      public:
        static bool DeleteFile(const Path& path) { return std::filesystem::remove(s_RootPath / path); }
        static bool FileExists(const Path& path) { return std::filesystem::exists(s_RootPath / path); }
        static bool CreateDirectories(const Path& path) { return std::filesystem::create_directories(s_RootPath / path); }

        static bool OpenFile(std::ifstream& stream, const Path& path, std::ios_base::openmode = std::ios::in)
        {
            stream.open(s_RootPath / path);
            if (stream.fail())
            {
                EVA_INTERNAL_ERROR("Failed to open file: {}", ToString(path));
                return false;
            }
            return true;
        }

        static bool OpenFile(std::ofstream& stream, const Path& path, std::ios_base::openmode = std::ios::out)
        {
            stream.open(s_RootPath / path);
            if (stream.fail())
            {
                EVA_INTERNAL_ERROR("Failed to open file: {}", ToString(path));
                return false;
            }
            return true;
        }

        static std::string ToString(const Path& path)
        {
            auto pathString = path.string();
            std::replace(pathString.begin(), pathString.end(), '\\', '/');
            return pathString;
        }

        static Path ToSystemPath(const Path& path)
        {
            return s_RootPath / path;
        }
    };
} // namespace EVA
