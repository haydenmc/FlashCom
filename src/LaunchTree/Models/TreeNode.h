#pragma once
#include <memory>
#include <span>
#include <string>

namespace LaunchTree::Models
{
    class TreeNode
    {
    public:
        TreeNode(
            uint32_t vkCode,
            std::wstring_view name,
            std::vector<std::unique_ptr<TreeNode>>& children
        );
        uint32_t VkCode();
        std::wstring Name();
        const std::vector<TreeNode*> Children();

    private:
        const uint32_t m_vkCode;
        const std::wstring m_name;
        const std::vector<std::unique_ptr<TreeNode>> m_children;
    };
}