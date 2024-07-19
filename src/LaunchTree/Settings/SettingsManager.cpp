#include <pch.h>
#include "SettingsManager.h"

#include "../Models/ActivateAumidTreeNode.h"
#include "../Models/LaunchUriTreeNode.h"
#include "../Models/ShellExecuteTreeNode.h"
#include <fstream>
#include <string_view>

namespace
{
    constexpr std::string_view c_settingsFileName{ "settings.json" };
    constexpr std::string_view c_defaultSettingsFileContents{ R"({
    "commandTree": [
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
                    "executeParameters": ""
                },
                {
                    "name": "Notepad",
                    "key": "N",
                    "type": "shellExecute",
                    "executeFile": "notepad.exe",
                    "executeParameters": ""
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
            "executeParameters": ""
        }
    ]
})" };

    std::filesystem::path GetSettingsFilePath()
    {
        return std::filesystem::path{ winrt::WStorage::ApplicationData::Current()
            .LocalFolder().Path().c_str() } / c_settingsFileName;
    }

    std::unique_ptr<LaunchTree::Models::TreeNode> ParseToTreeNode(const nlohmann::json& json)
    {
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>> nodeChildren;

        // Basic validation
        if (!json.contains("name") || !json.at("name").is_string() ||
            !json.contains("key") || !json.at("key").is_string())
        {
            // TODO: Log warning
            return nullptr;
        }

        auto nodeName{ json.at("name").get<std::string>() };
        auto jsonKeyCodeString{ json.at("key").get<std::string>() };
        uint32_t nodeKeyCode{ static_cast<uint32_t>(std::toupper(jsonKeyCodeString.at(0))) };
        
        if (json.contains("children") && json.at("children").is_array())
        {
            for (auto& childJson : json.at("children"))
            {
                if (auto childNode{ ParseToTreeNode(childJson) })
                {
                    nodeChildren.push_back(std::move(childNode));
                }
            }
        }

        if (json.contains("type") && json.at("type").is_string())
        {
            auto nodeType{ json.at("type").get<std::string>() };
            if (nodeType == "shellExecute")
            {
                std::string executeParameters;
                auto executeFile{ json.at("executeFile").get<std::string>() };
                if (json.contains("executeParameters") && json.at("executeParameters").is_string())
                {
                    executeParameters = json.at("executeParameters").get<std::string>();
                }
                return std::make_unique<LaunchTree::Models::ShellExecuteTreeNode>(nodeKeyCode,
                    nodeName, executeFile, executeParameters);
            }
            else if (nodeType == "uri")
            {
                auto uri{ json.at("uri").get<std::string>() };
                return std::make_unique<LaunchTree::Models::LaunchUriTreeNode>(nodeKeyCode,
                    nodeName, uri);
            }
            else if (nodeType == "aumid")
            {
                auto aumid{ json.at("aumid").get<std::string>() };
                return std::make_unique<LaunchTree::Models::ActivateAumidTreeNode>(nodeKeyCode,
                    nodeName, aumid);
            }
            else
            {
                // TODO: LOG ERROR
            }
        }

        return std::make_unique<LaunchTree::Models::TreeNode>(nodeKeyCode, nodeName,
            std::move(nodeChildren));
    }
}

namespace LaunchTree::Settings
{
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

    std::filesystem::path SettingsManager::GetSettingsFilePath()
    {
        return m_settingsFilePath;
    }

    std::unique_ptr<Models::TreeNode> SettingsManager::GetTree()
    {
        std::vector<std::unique_ptr<Models::TreeNode>> rootChildren;
        std::ifstream settingsFileStream{ m_settingsFilePath };
        const auto settingsJson{ nlohmann::json::parse(settingsFileStream) };
        if (settingsJson.contains("commandTree"))
        {
            const auto& commandTreeJson{ settingsJson.at("commandTree") };
            commandTreeJson.is_array();
            for (const auto& commandTreeObject : commandTreeJson)
            {
                auto treeNode{ ParseToTreeNode(commandTreeObject) };
                rootChildren.push_back(std::move(treeNode));
            }
        }
        else
        {
            // Log error
        }

        return std::make_unique<Models::TreeNode>(0, "Root", std::move(rootChildren));
    }
}
