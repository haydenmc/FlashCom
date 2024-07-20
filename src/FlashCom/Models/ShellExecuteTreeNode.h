#pragma once
#include "TreeNode.h"

namespace FlashCom::Models
{
struct ShellExecuteTreeNode : public TreeNode
{
    ShellExecuteTreeNode(uint32_t vkCode, std::string_view name, std::string_view file,
        std::string_view parameters);
    void Execute() const override;
private:
    const std::string m_file;
    const std::string m_parameters;
};
}