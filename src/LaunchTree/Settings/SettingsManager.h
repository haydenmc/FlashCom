#pragma once
#include <filesystem>
#include "../Models/TreeNode.h"

namespace LaunchTree::Settings
{
    struct SettingsManager
    {
        SettingsManager();
        std::unique_ptr<Models::TreeNode> GetTree();

    private:
        std::filesystem::path const m_settingsFilePath;
    };
}