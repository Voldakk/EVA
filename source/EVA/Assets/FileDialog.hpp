#pragma once

namespace EVA
{
    class FileDialog
    {
        inline static std::unordered_map<std::string, std::string> s_Extentions;

      public:
        static void Register(const std::string& extention, const std::string& name);

        static std::vector<std::filesystem::path> OpenFile(const std::vector<std::string>& filter, bool multiSelect = false);
        static std::filesystem::path SaveFile(const std::string& extention);
    };
} // namespace EVA
