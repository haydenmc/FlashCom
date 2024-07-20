#pragma once
#include <winuser.h>

#include <functional>

namespace FlashCom::Input
{
    void SetGlobalLowLevelKeyboardCallback(std::function<void(WPARAM, KBDLLHOOKSTRUCT*)> callback);
}
