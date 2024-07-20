#include <pch.h>
#include "LaunchUriTreeNode.h"

namespace FlashCom::Models
{
    LaunchUriTreeNode::LaunchUriTreeNode(uint32_t vkCode, std::string_view name,
        std::string_view uri) : TreeNode{ vkCode, name, {} }, m_uri{ uri }
    { }

    void LaunchUriTreeNode::Execute() const
    {
        winrt::WF::Uri uri{ winrt::to_hstring(m_uri) };
        winrt::WS::Launcher::LaunchUriAsync(uri);
    }
}