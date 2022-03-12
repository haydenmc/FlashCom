#include <pch.h>
#include "TreeNode.h"

#include <ranges>

namespace LaunchTree::Models
{
    TreeNode::TreeNode(
        uint32_t vkCode,
        std::wstring_view name,
        std::vector<std::unique_ptr<TreeNode>>& children
    ) :
        m_vkCode{ vkCode },
        m_name{ name },
        m_children{ std::move(children) }
    { }

    uint32_t TreeNode::VkCode()
    {
        return m_vkCode;
    }

    std::wstring TreeNode::Name()
    {
        return m_name;
    }

    const std::vector<TreeNode*> TreeNode::Children()
    {
        std::vector<TreeNode*> returnValue;
        returnValue.reserve(m_children.size());
        std::transform(
            m_children.begin(),
            m_children.end(),
            returnValue.begin(),
            [](std::unique_ptr<TreeNode> in) {
                return in.get();
            }
        );
        return returnValue;
    }
}