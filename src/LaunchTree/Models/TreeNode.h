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
            std::string_view name,
            std::vector<std::unique_ptr<TreeNode>>&& children
        );
        uint32_t GetVkCode() const;
        std::string GetName() const;
        const std::vector<TreeNode*> GetChildren() const;
        virtual void Execute() const;

    private:
        const uint32_t m_vkCode;
        const std::string m_name;
        const std::vector<std::unique_ptr<TreeNode>> m_children;
    };
}