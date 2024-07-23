#pragma once
#include <expected>
#include <filesystem>
#include <shared_mutex>
#include "../Models/TreeNode.h"

namespace FlashCom::Settings
{
    std::filesystem::path GetApplicationLocalDataDirectory();

    struct SettingsManager
    {
        SettingsManager();
        std::expected<void, std::string> LoadSettings();
        std::filesystem::path GetSettingsFilePath();
        std::shared_ptr<Models::TreeNode> GetCommandTreeRoot();

    private:
        std::filesystem::path const m_settingsFilePath;
        std::shared_mutex m_settingsAccessMutex;
        std::shared_ptr<Models::TreeNode> m_commandTreeRoot;

        std::expected<void, std::string> PopulateCommandTree(
            const std::unique_lock<std::shared_mutex>& accessLock,
            const nlohmann::json& settingsJson);
    };
}