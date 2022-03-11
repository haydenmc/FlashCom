#pragma once
#include <winuser.h>

#include <functional>

namespace LaunchTree::Input
{
    void SetGlobalLowLevelKeyboardCallback(std::function<void(WPARAM, KBDLLHOOKSTRUCT*)> callback);
}
