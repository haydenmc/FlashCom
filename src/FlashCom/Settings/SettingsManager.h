#pragma once
#include <filesystem>
#include "../Models/TreeNode.h"

namespace FlashCom::Settings
{
    struct SettingsManager
    {
        SettingsManager();
        std::filesystem::path GetSettingsFilePath();
        std::unique_ptr<Models::TreeNode> GetTree();

    private:
        std::filesystem::path const m_settingsFilePath;
    };
}