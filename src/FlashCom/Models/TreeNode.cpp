#include <pch.h>
#include "TreeNode.h"

#include <iterator>

namespace FlashCom::Models
{
    TreeNode::TreeNode(
        uint32_t vkCode,
        std::string_view name,
        std::vector<std::unique_ptr<TreeNode>>&& children
    ) :
        m_vkCode{ vkCode },
        m_name{ name },
        m_children{ std::move(children) }
    { }

    uint32_t TreeNode::GetVkCode() const
    {
        return m_vkCode;
    }

    std::string TreeNode::GetName() const
    {
        return m_name;
    }

    const std::vector<TreeNode*> TreeNode::GetChildren() const
    {
        std::vector<TreeNode*> returnValue;
        returnValue.reserve(m_children.size());
        std::transform(
            m_children.begin(),
            m_children.end(),
            std::back_inserter(returnValue),
            [](const std::unique_ptr<TreeNode>& in)
            {
                return in.get();
            }
        );
        return returnValue;
    }

    void TreeNode::Execute() const
    {
        // TODO
    }
}