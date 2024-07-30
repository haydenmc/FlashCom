#include <pch.h>
#include "ShellExecuteTreeNode.h"

namespace
{
    std::optional<std::filesystem::path> GetExecuteDirectory(std::string_view file,
        std::string_view directory)
    {
        if (directory.length() > 0)
        {
            return directory;
        }

        // Default to the folder containing the file if one exists
        std::filesystem::path filePath{ file };
        if (filePath.has_parent_path())
        {
            auto parentPath{ filePath.parent_path() };
            SPDLOG_INFO("::GetExecuteDirectory - No execute directory defined for '{}', "
                "defaulting to '{}'", file, parentPath.string());
            return parentPath;
        }

        SPDLOG_INFO("::GetExecuteDirectory - No execute directory defined for '{}', "
            "defaulting to null.", file);
        return std::nullopt;
    }
}

namespace FlashCom::Models
{
    ShellExecuteTreeNode::ShellExecuteTreeNode(uint32_t vkCode, std::string_view name,
        std::string_view file, std::string_view parameters, std::string_view directory) :
        TreeNode{ vkCode, name, {} }, m_file{ file }, m_parameters{ parameters },
        m_directory{ GetExecuteDirectory(file, directory) }
    { }
    void ShellExecuteTreeNode::Execute() const
    {
        auto fileString{ utf8::utf8to16(m_file) };
        auto parametersString{ utf8::utf8to16(m_parameters) };
        auto executeResult{ ShellExecuteW(
            nullptr,
            L"open",
            reinterpret_cast<wchar_t*>(fileString.data()),
            reinterpret_cast<wchar_t*>(parametersString.data()),
            m_directory.has_value() ?
                reinterpret_cast<wchar_t*>(m_directory.value().wstring().data()) : nullptr,
            SW_SHOWNORMAL
        ) };
        if (reinterpret_cast<INT_PTR>(executeResult) <= 32)
        {
            winrt::throw_last_error();
        }
    }
}