#include <pch.h>
#include "TreeNode.h"

#include <iterator>

namespace LaunchTree::Models
{
    TreeNode::TreeNode(
        uint32_t vkCode,
        std::wstring_view name,
        std::vector<std::unique_ptr<TreeNode>>&& children
    ) :
        m_vkCode{ vkCode },
        m_name{ name },
        m_children{ std::move(children) }
    { }

    uint32_t TreeNode::GetVkCode()
    {
        return m_vkCode;
    }

    std::wstring TreeNode::GetName()
    {
        return m_name;
    }

    const std::vector<TreeNode*> TreeNode::GetChildren()
    {
        std::vector<TreeNode*> returnValue;
        returnValue.reserve(m_children.size());
        std::transform(
            m_children.begin(),
            m_children.end(),
            returnValue.begin(),
            [](const std::unique_ptr<TreeNode>& in)
            {
                return in.get();
            }
        );
        return returnValue;
    }

    void TreeNode::Execute()
    {
        // TODO
    }
}