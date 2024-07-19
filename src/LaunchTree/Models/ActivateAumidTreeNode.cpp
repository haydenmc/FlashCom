#include <pch.h>
#include "ActivateAumidTreeNode.h"

#include <shobjidl_core.h>

namespace LaunchTree::Models
{
    ActivateAumidTreeNode::ActivateAumidTreeNode(uint32_t vkCode, std::string_view name,
        std::string_view aumid) : TreeNode{ vkCode, name, {} }, m_aumid{ aumid }
    { }

    void ActivateAumidTreeNode::Execute() const
    {
        auto activationManager{ winrt::try_create_instance<IApplicationActivationManager>(
            CLSID_ApplicationActivationManager) };
        DWORD pid{ 0 };
        winrt::check_hresult(activationManager->ActivateApplication(
            reinterpret_cast<wchar_t*>(utf8::utf8to16(m_aumid).data()), nullptr, AO_NONE, &pid));
    }
}