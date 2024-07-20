#pragma once
#include <winuser.h>

#include <functional>

namespace FlashCom::Input
{
    enum class LowLevelCallbackReturnKind
    {
        Handled,
        Unhandled,
    };
    void SetGlobalLowLevelKeyboardCallback(
        std::function<LowLevelCallbackReturnKind(WPARAM, KBDLLHOOKSTRUCT*)> callback);
}
