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
        bool GetShowStartupNotification();
        bool UseTwentyFourHourClock();
        std::shared_ptr<Models::TreeNode> GetCommandTreeRoot();

    private:
        std::filesystem::path const m_settingsFilePath;
        std::shared_mutex m_settingsAccessMutex;
        bool m_showStartupNotification{ true };
        bool m_useTwentyFourHourClock{ false };
        std::shared_ptr<Models::TreeNode> m_commandTreeRoot;

        void PopulateSettingsValues(
            const std::unique_lock<std::shared_mutex>& accessLock,
            const nlohmann::json& settingsJson);
        std::expected<void, std::string> PopulateCommandTree(
            const std::unique_lock<std::shared_mutex>& accessLock,
            const nlohmann::json& settingsJson);
    };
}