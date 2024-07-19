#pragma once
#include "TreeNode.h"

namespace LaunchTree::Models
{
struct ActivateAumidTreeNode : public TreeNode
{
    ActivateAumidTreeNode(uint32_t vkCode, std::string_view name, std::string_view aumid);
    void Execute() const override;
private:
    std::string const m_aumid;
};
}