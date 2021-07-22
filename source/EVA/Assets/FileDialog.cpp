#include "FileDialog.hpp"
#include <portable-file-dialogs.h>

namespace EVA
{
    void FileDialog::Register(const std::string& extention, const std::string& name) { s_Extentions[extention] = name; }

    std::vector<std::filesystem::path> FileDialog::OpenFile(const std::vector<std::string>& filter, bool multiSelect)
    {
        pfd::opt opt = multiSelect ? pfd::opt::multiselect : pfd::opt::none;
        auto res     = pfd::open_file("Open file", ".", filter, opt).result();
        std::vector<std::filesystem::path> paths;
        for (const auto& s : res)
        {
            paths.push_back(std::filesystem::path(s));
        }
        return paths;
    }

    std::filesystem::path FileDialog::SaveFile(const std::string& extention)
    {
        auto it = s_Extentions.find(extention);
        EVA_INTERNAL_ASSERT(it != s_Extentions.end(), "Extention \"{}\" is not registered", extention);
        auto extentionName = (*it).second;

        std::string res = pfd::save_file("Save file", ".", {extentionName, "*" + extention}, pfd::opt::force_overwrite).result();

        auto path = std::filesystem::path(res);
        if (!path.empty()) { path.replace_extension(std::filesystem::path(extention)); }
        return path;
    }
} // namespace EVA
