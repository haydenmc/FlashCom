#include <pch.h>
#include "ShellExecuteTreeNode.h"

namespace FlashCom::Models
{
    ShellExecuteTreeNode::ShellExecuteTreeNode(uint32_t vkCode, std::string_view name,
        std::string_view file, std::string_view parameters) : TreeNode{ vkCode, name, {} },
        m_file{ file }, m_parameters{ parameters }
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
            nullptr,
            SW_SHOWNORMAL
        ) };
        if (reinterpret_cast<INT_PTR>(executeResult) <= 32)
        {
            winrt::throw_last_error();
        }
    }
}