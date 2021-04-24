#pragma once

namespace EVA::FileSystem
{
    static std::string ToString(const std::filesystem::path& path)
    {
        auto pathString = path.string();
        std::replace(pathString.begin(), pathString.end(), '\\', '/');
        return pathString;
    }
} // namespace EVA::FileSystem
