#pragma once
#include "TreeNode.h"

namespace LaunchTree::Models
{
struct LaunchUriTreeNode : public TreeNode
{
    LaunchUriTreeNode(uint32_t vkCode, std::string_view name, std::string_view uri);
    void Execute() const override;
private:
    std::string const m_uri;
};
}