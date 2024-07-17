#include <pch.h>
#include "SettingsManager.h"

#include <string_view>

namespace
{
    constexpr std::wstring_view c_settingsFileName{ L"settings.json" };

    std::filesystem::path GetSettingsFilePath()
    {
        return std::filesystem::path{ winrt::Windows::Storage::ApplicationData::Current()
            .LocalFolder().Path().c_str() } / c_settingsFileName;
    }
}

namespace LaunchTree::Settings
{
    SettingsManager::SettingsManager() : m_settingsFilePath{ GetSettingsFilePath() }
    { }

    std::unique_ptr<Models::TreeNode> SettingsManager::GetTree()
    {
        return std::unique_ptr<Models::TreeNode>();
    }
}
