#pragma once

namespace EVA
{
    using Path = std::filesystem::path;

    class FileSystem
    {
        inline static std::filesystem::path s_RootPath = "./assets/";

      public:
        static bool Delete(const Path& path) { return std::filesystem::remove(s_RootPath / path); }
        static bool Exists(const Path& path) { return std::filesystem::exists(s_RootPath / path); }
        static bool CreateDirectories(const Path& path) { return std::filesystem::create_directories(s_RootPath / path); }

        static bool OpenFile(std::ifstream& stream, const Path& path, std::ios_base::openmode mode = std::ios::in)
        {
            stream.open(s_RootPath / path, mode);
            if (stream.fail())
            {
                EVA_INTERNAL_ERROR("Failed to open file: {}", ToString(path));
                return false;
            }
            return true;
        }

        static bool OpenFile(std::ofstream& stream, const Path& path, std::ios_base::openmode mode = std::ios::out)
        {
            stream.open(s_RootPath / path, mode);
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

        static std::string ToSafeString(const Path& path)
        {
            auto pathString = path.string();
            std::replace(pathString.begin(), pathString.end(), '\\', '_');
            std::replace(pathString.begin(), pathString.end(), '/', '_');
            return pathString;
        }

        static Path ToSystemPath(const Path& path)
        {
            return s_RootPath / path;
        }

        static bool ReadFile(const Path& path, std::string& buffer)
        {
            std::ifstream file;
            if (OpenFile(file, path, std::ios::in | std::ios::binary))
            {
                buffer = {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
                file.close();
                return true;
            }
            return false;
        }

        static bool WriteFile(const Path& path, const std::string& data)
        {
            std::ofstream file;
            if (OpenFile(file, path, std::ios::out | std::ios::trunc))
            {
                file.write((char*)data.data(), data.size() * sizeof(std::string::value_type));
                file.flush();
                file.close();
                return true;
            }
            return false;
        }

        template<typename T>
        static bool ReadFile(const Path& path, std::vector<T>& buffer)
        {
            std::ifstream file;
            if (OpenFile(file, path, std::ios::in | std::ios::binary))
            {
                /*file.ignore(std::numeric_limits<std::streamsize>::max());
                std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);*/

                auto size = std::filesystem::file_size(s_RootPath / path);

                buffer.resize(size / sizeof(T));

                file.read((char*)buffer.data(), size);
                file.close();
                return true;
            }
            return false;
        }

        template<typename T>
        static bool WriteFile(const Path& path, const std::vector<T>& data)
        {
            std::ofstream file;
            if (OpenFile(file, path, std::ios::out | std::ios::binary | std::ios::trunc))
            {
                file.write((char*)data.data(), data.size() * sizeof(T));
                file.flush();
                file.close();
                return true;
            }
            return false;
        }
    };
} // namespace EVA
