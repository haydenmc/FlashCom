#include <pch.h>
#include "SettingsManager.h"

#include "../Models/ActivateAumidTreeNode.h"
#include "../Models/LaunchUriTreeNode.h"
#include "../Models/ShellExecuteTreeNode.h"
#include <fstream>
#include <ShlObj_core.h>
#include <string_view>

namespace
{
    constexpr std::string_view c_appDataLocalDirectoryName{ "FlashCom" };
    constexpr std::string_view c_settingsFileName{ "settings.json" };
    // JSON property names
    constexpr std::string_view c_showStartupNotificationProperty{ "showStartupNotification" };
    constexpr std::string_view c_useTwentyFourHourClockProperty{ "use24HourClock" };
    constexpr std::string_view c_commandsJsonProperty{ "commands" };
    constexpr std::string_view c_commandNameJsonProperty{ "name" };
    constexpr std::string_view c_commandKeyJsonProperty{ "key" };
    constexpr std::string_view c_commandChildrenJsonProperty{ "children" };
    constexpr std::string_view c_commandTypeJsonProperty{ "type" };
    constexpr std::string_view c_commandExecuteFileJsonProperty{ "executeFile" };
    constexpr std::string_view c_commandExecuteParametersJsonProperty{ "executeParameters" };
    constexpr std::string_view c_commandExecuteDirectoryJsonProperty{ "executeDirectory" };
    constexpr std::string_view c_commandUriJsonProperty{ "uri" };
    constexpr std::string_view c_commandAumidJsonProperty{ "aumid" };
    // JSON command node 'type' values
    constexpr std::string_view c_commandTypeValueShellExecute{ "shellExecute" };
    constexpr std::string_view c_commandTypeValueUri{ "uri" };
    constexpr std::string_view c_commandTypeValueAumid{ "aumid" };
    // Default settings.json contents
    constexpr std::string_view c_defaultSettingsFileContents{ R"({
    "showStartupNotification": true,
    "use24HourClock": false,
    "commands": [
        {
            "name": "README",
            "key": "R",
            "type": "uri",
            "uri": "https://github.com/haydenmc/FlashCom/blob/main/README.md"
        },
        {
            "name": "Tools",
            "key": "T",
            "children": [
                {
                    "name": "Alarms + Clock",
                    "key": "A",
                    "type": "aumid",
                    "aumid": "Microsoft.WindowsAlarms_8wekyb3d8bbwe!App"
                },
                {
                    "name": "Calculator",
                    "key": "C",
                    "type": "shellExecute",
                    "executeFile": "calc.exe",
                    "executeParameters": "",
                    "executeDirectory": ""
                },
                {
                    "name": "Notepad",
                    "key": "N",
                    "type": "shellExecute",
                    "executeFile": "notepad.exe",
                    "executeParameters": "",
                    "executeDirectory": ""
                },
                {
                    "name": "Settings",
                    "key": "S",
                    "type": "uri",
                    "uri": "ms-settings:"
                }
            ]
        },
        {
            "name": "Edge",
            "key": "E",
            "type": "shellExecute",
            "executeFile": "msedge.exe",
            "executeParameters": "",
            "executeDirectory": ""
        }
    ]
})" };

    std::filesystem::path GetSettingsFilePath()
    {
        return FlashCom::Settings::GetApplicationLocalDataDirectory() / c_settingsFileName;
    }

    bool IsValidKeyString(const std::string& key)
    {
        // Valid keys map to single vkey codes as defined in winuser.h and are not
        // special characters or modifier keys.
        // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
        if (key.size() != 1)
        {
            return false;
        }

        auto keyCode{ static_cast<uint8_t>(std::toupper(key.at(0))) };
        if ((keyCode < 0x30) ||
            (keyCode > 0x39 && keyCode < 0x41) ||
            (keyCode > 0x5A))
        {
            return false;
        }

        return true;
    }

    std::expected<std::unique_ptr<FlashCom::Models::ShellExecuteTreeNode>, std::string>
        ParseToShellExecuteTreeNode(uint8_t keyCode, const std::string& name,
            const nlohmann::json& json)
    {
        std::string executeFile;
        std::string executeParameters;
        std::string executeDirectory;
        if (!json.contains(c_commandExecuteFileJsonProperty) ||
            !json.at(c_commandExecuteFileJsonProperty).is_string())
        {
            SPDLOG_ERROR("::ParseToShellExecuteTreeNode - Command '{}' type '{}' "
                "has no valid '{}' string property", name, c_commandTypeValueShellExecute,
                c_commandExecuteFileJsonProperty);
            return std::unexpected(std::format(
                "Command '{}' type '{}' has no valid '{}' string property",
                name, c_commandTypeValueShellExecute, c_commandExecuteFileJsonProperty));
        }
        executeFile = json.at(c_commandExecuteFileJsonProperty).get<std::string>();

        // executeParameters is optional
        if (json.contains(c_commandExecuteParametersJsonProperty) &&
            json.at(c_commandExecuteParametersJsonProperty).is_string())
        {
            executeParameters = json.at(c_commandExecuteParametersJsonProperty).get<std::string>();
        }

        // executeDirectory is optional
        if (json.contains(c_commandExecuteDirectoryJsonProperty) &&
            json.at(c_commandExecuteDirectoryJsonProperty).is_string())
        {
            executeDirectory = json.at(c_commandExecuteDirectoryJsonProperty).get<std::string>();
        }

        SPDLOG_INFO("::ParseToShellExecuteTreeNode - Creating ShellExecute node {}:{}, "
            "file: '{}', parameters: '{}', directory: '{}'", static_cast<char>(keyCode), name,
            executeFile, executeParameters, executeDirectory);
        return std::make_unique<FlashCom::Models::ShellExecuteTreeNode>(keyCode,
            name, executeFile, executeParameters, executeDirectory);
    }

    std::expected<std::unique_ptr<FlashCom::Models::LaunchUriTreeNode>, std::string>
        ParseToLaunchUriTreeNode(uint8_t keyCode, const std::string& name,
            const nlohmann::json& json)
    {
        if (!json.contains(c_commandUriJsonProperty) ||
            !json.at(c_commandUriJsonProperty).is_string())
        {
            SPDLOG_ERROR("::ParseToLaunchUriTreeNode - Command '{}' type '{}' "
                "has no valid '{}' string property", name, c_commandTypeValueUri,
                c_commandUriJsonProperty);
            return std::unexpected(std::format(
                "Command '{}' type '{}' has no valid '{}' string property",
                name, c_commandTypeValueUri, c_commandUriJsonProperty));
        }
        auto uri{ json.at(c_commandUriJsonProperty).get<std::string>() };
        SPDLOG_INFO("::ParseToLaunchUriTreeNode - Creating LaunchUri node {}:{}, uri: '{}'",
            static_cast<char>(keyCode), name, uri);
        return std::make_unique<FlashCom::Models::LaunchUriTreeNode>(keyCode, name, uri);
    }

    std::expected<std::unique_ptr<FlashCom::Models::ActivateAumidTreeNode>, std::string>
        ParseToActivateAumidTreeNode(uint8_t keyCode, const std::string& name,
            const nlohmann::json& json)
    {
        if (!json.contains(c_commandAumidJsonProperty) ||
            !json.at(c_commandAumidJsonProperty).is_string())
        {
            SPDLOG_ERROR("::ParseToActivateAumidTreeNode - Command '{}' type '{}' "
                "has no valid '{}' string property", name, c_commandTypeValueAumid,
                c_commandAumidJsonProperty);
            return std::unexpected(std::format(
                "Command '{}' type '{}' has no valid '{}' string property",
                name, c_commandTypeValueAumid, c_commandAumidJsonProperty));
        }
        auto aumid{ json.at(c_commandAumidJsonProperty).get<std::string>() };
        SPDLOG_INFO(
            "::ParseToActivateAumidTreeNode - Creating ActivateAumid node {}:{}, aumid: '{}'",
            static_cast<char>(keyCode), name, aumid);
        return std::make_unique<FlashCom::Models::ActivateAumidTreeNode>(keyCode, name, aumid);
    }

    std::expected<std::unique_ptr<FlashCom::Models::TreeNode>, std::string> ParseToTreeNode(
        const nlohmann::json& json)
    {
        std::vector<std::unique_ptr<FlashCom::Models::TreeNode>> nodeChildren;

        if (!json.contains(c_commandNameJsonProperty) ||
            !json.at(c_commandNameJsonProperty).is_string() ||
            !json.contains(c_commandKeyJsonProperty) ||
            !json.at(c_commandKeyJsonProperty).is_string())
        {
            SPDLOG_ERROR("::ParseToTreeNode - no valid '{}' or '{}' string properties found",
                c_commandNameJsonProperty, c_commandKeyJsonProperty);
            return std::unexpected(std::format(
                "Command object has no '{}' or '{}' string properties",
                c_commandNameJsonProperty, c_commandKeyJsonProperty));
        }

        auto nodeName{ json.at(c_commandNameJsonProperty).get<std::string>() };
        auto jsonKeyCodeString{ json.at(c_commandKeyJsonProperty).get<std::string>() };
        if (!IsValidKeyString(jsonKeyCodeString))
        {
            SPDLOG_ERROR("::ParseToTreeNode - invalid '{}' value '{}', "
                "must be single alphanumeric keyboard key", c_commandKeyJsonProperty,
                jsonKeyCodeString);
            return std::unexpected(std::format(
                "Command object has invalid '{}' value '{}', "
                "must be single alphanumeric keyboard key", c_commandKeyJsonProperty,
                jsonKeyCodeString));
        }
        auto nodeKeyCode{ static_cast<uint8_t>(std::toupper(jsonKeyCodeString.at(0))) };

        if (json.contains(c_commandChildrenJsonProperty))
        {
            if (!json.at(c_commandChildrenJsonProperty).is_array())
            {
                SPDLOG_ERROR("::ParseToTreeNode - '{}' must be an array of child commands",
                    c_commandChildrenJsonProperty);
                return std::unexpected(std::format(
                    "Command object has invalid '{}' type, must be an array of child commands.",
                    c_commandChildrenJsonProperty));
            }

            for (auto& childJson : json.at(c_commandChildrenJsonProperty))
            {
                auto childNode{ ParseToTreeNode(childJson) };
                if (childNode.has_value())
                {
                    nodeChildren.push_back(std::move(childNode.value()));
                }
                else
                {
                    SPDLOG_WARN("::ParseToTreeNode - Skipping child node of '{}': {}",
                        nodeName, childNode.error());
                }
            }
        }

        if (json.contains(c_commandTypeJsonProperty) &&
            json.at(c_commandTypeJsonProperty).is_string())
        {
            auto nodeType{ json.at(c_commandTypeJsonProperty).get<std::string>() };
            if (json.contains(c_commandChildrenJsonProperty))
            {
                SPDLOG_WARN("::ParseToTreeNode - Command '{}' contains both '{}' and '{}' "
                    "properties. Skipping '{}'...", nodeName, c_commandChildrenJsonProperty,
                    c_commandTypeJsonProperty, c_commandTypeJsonProperty);
            }
            else if (nodeType == c_commandTypeValueShellExecute)
            {
                return ParseToShellExecuteTreeNode(nodeKeyCode, nodeName, json);
            }
            else if (nodeType == c_commandTypeValueUri)
            {
                return ParseToLaunchUriTreeNode(nodeKeyCode, nodeName, json);
            }
            else if (nodeType == c_commandTypeValueAumid)
            {
                return ParseToActivateAumidTreeNode(nodeKeyCode, nodeName, json);
            }
            else
            {
                SPDLOG_ERROR(
                    "::ParseToTreeNode - Command '{}' contains an unknown '{}' value '{}'",
                    nodeName, c_commandTypeJsonProperty, nodeType);
                return std::unexpected(std::format(
                    "Command '{}' contains an unknown '{}' value '{}'",
                    nodeName, c_commandTypeJsonProperty, nodeType));
            }
        }

        SPDLOG_INFO("::ParseToTreeNode - Creating node {}:{} with {} children",
            static_cast<char>(nodeKeyCode), nodeName, nodeChildren.size());
        return std::make_unique<FlashCom::Models::TreeNode>(nodeKeyCode, nodeName,
            std::move(nodeChildren));
    }
}

namespace FlashCom::Settings
{
    std::filesystem::path GetApplicationLocalDataDirectory()
    {
        static std::optional<std::filesystem::path> appLocalDataDir;
        if (appLocalDataDir.has_value())
        {
            return appLocalDataDir.value();
        }

        // If this app is running inside of a Windows Package, we should use the appropriate
        // app package data directory.
        try
        {
            appLocalDataDir = winrt::WStorage::ApplicationData::Current()
                .LocalFolder().Path().c_str();
            return appLocalDataDir.value();
        }
        catch (const winrt::hresult_error& e)
        {
            // The only error we expect to see here is "process has no package identity".
            if (e.code() != 0x80073D54)
            {
                throw e;
            }
        }
        // Otherwise, just use %LOCALAPPDATA%
        wil::unique_cotaskmem_string appDataFolderString;
        winrt::check_hresult(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr,
            appDataFolderString.put()));
        appLocalDataDir = (std::filesystem::path{ appDataFolderString.get() } /
            c_appDataLocalDirectoryName);
        return appLocalDataDir.value();
    }

    SettingsManager::SettingsManager() : m_settingsFilePath{ ::GetSettingsFilePath() }
    {
        if (!std::filesystem::exists(m_settingsFilePath))
        {
            SPDLOG_INFO("SettingsManager::SettingsManager - Writing default settings file to {}",
                m_settingsFilePath.string());
            std::ofstream defaultSettingsFile{ m_settingsFilePath };
            defaultSettingsFile << c_defaultSettingsFileContents;
            defaultSettingsFile.close();
        }
    }

    std::expected<void, std::string> SettingsManager::LoadSettings()
    {
        SPDLOG_INFO("SettingsManager::LoadSettings - Loading settings from {}...",
            m_settingsFilePath.string());
        std::unique_lock settingsLock{ m_settingsAccessMutex };
        std::ifstream settingsFileStream{};
        nlohmann::json settingsJson{};
        try
        {
            settingsFileStream = std::ifstream{ m_settingsFilePath };
            settingsJson = nlohmann::json::parse(settingsFileStream);
        }
        catch (const nlohmann::json::exception& e)
        {
            SPDLOG_ERROR("SettingsManager::LoadSettings - Could not parse settings.json file: {}",
                e.what());
            return std::unexpected(std::format("Could not parse settings.json file: {}",
                e.what()));
        }
        PopulateSettingsValues(settingsLock, settingsJson);
        PopulateCommandTree(settingsLock, settingsJson);
        return {}; // Spurious warning C4715
    }

    std::filesystem::path SettingsManager::GetSettingsFilePath()
    {
        std::shared_lock settingsLock{ m_settingsAccessMutex };
        return m_settingsFilePath;
    }

    bool SettingsManager::GetShowStartupNotification()
    {
        return m_showStartupNotification;
    }

    bool SettingsManager::UseTwentyFourHourClock()
    {
        return m_useTwentyFourHourClock;
    }

    std::shared_ptr<Models::TreeNode> SettingsManager::GetCommandTreeRoot()
    {
        std::shared_lock settingsLock{ m_settingsAccessMutex };
        return m_commandTreeRoot;
    }

    void SettingsManager::PopulateSettingsValues(
        const std::unique_lock<std::shared_mutex>& /*accessLock*/,
        const nlohmann::json& settingsJson)
    {
        SPDLOG_INFO("SettingsManager::PopulateSettingsValues");

        if (settingsJson.contains(c_showStartupNotificationProperty) &&
            settingsJson.at(c_showStartupNotificationProperty).is_boolean())
        {
            m_showStartupNotification =
                settingsJson.at(c_showStartupNotificationProperty).get<bool>();
        }
        else
        {
            SPDLOG_INFO("SettingsManager::PopulateSettingsValues - No '{}' bool property found. "
                "Defaulting to '{}'.", c_showStartupNotificationProperty,
                m_showStartupNotification);
        }

        if (settingsJson.contains(c_useTwentyFourHourClockProperty) &&
            settingsJson.at(c_useTwentyFourHourClockProperty).is_boolean())
        {
            m_useTwentyFourHourClock =
                settingsJson.at(c_useTwentyFourHourClockProperty).get<bool>();
        }
        else
        {
            SPDLOG_INFO("SettingsManager::PopulateSettingsValues - No '{}' bool property found. "
                "Defaulting to '{}'.", c_useTwentyFourHourClockProperty,
                m_useTwentyFourHourClock);
        }
    }

    std::expected<void, std::string> SettingsManager::PopulateCommandTree(
        const std::unique_lock<std::shared_mutex>& /*accessLock*/,
        const nlohmann::json& settingsJson)
    {
        SPDLOG_INFO("SettingsManager::PopulateCommandTree");
        m_commandTreeRoot = nullptr;

        if (!settingsJson.contains(c_commandsJsonProperty) ||
            !settingsJson.at(c_commandsJsonProperty).is_array())
        {
            SPDLOG_ERROR("SettingsManager::PopulateCommandTree - No '{}' array property found",
                c_commandsJsonProperty);
            return std::unexpected(std::format("No '{}' array property found.",
                c_commandsJsonProperty));
        }

        std::vector<std::unique_ptr<Models::TreeNode>> rootChildren;
        const auto& commandTreeJson{ settingsJson.at(c_commandsJsonProperty) };
        for (const auto& commandTreeObject : commandTreeJson)
        {
            auto rootChild{ ParseToTreeNode(commandTreeObject) };
            if (rootChild.has_value())
            {
                rootChildren.push_back(std::move(rootChild.value()));
            }
            else
            {
                SPDLOG_WARN("SettingsManager::PopulateCommandTree - Skipping root child: {}",
                    rootChild.error());
            }
        }

        SPDLOG_INFO("SettingsManager::PopulateCommandTree - Creating root node with {} children",
            rootChildren.size());
        m_commandTreeRoot = std::make_shared<Models::TreeNode>(0, "Root", std::move(rootChildren));
        return {}; // Spurious warning C4715
    }
}
