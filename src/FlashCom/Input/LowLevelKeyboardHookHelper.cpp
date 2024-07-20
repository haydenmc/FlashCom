#include <pch.h>
#include "LowLevelKeyboardHookHelper.h"

#include <shared_mutex>

namespace
{
    std::shared_mutex g_lowLevelKeyboardHookMutex;
    ::HHOOK g_hHook{ 0 };
    std::function<FlashCom::Input::LowLevelCallbackReturnKind(WPARAM, KBDLLHOOKSTRUCT*)>
        g_lowLevelKeyboardCallback;

    LRESULT LowLevelKeyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        // See https://learn.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc
        if (nCode < 0)
        {
            return ::CallNextHookEx(0, nCode, wParam, lParam);
        }
        bool callNextHook{ true };
        {
            std::shared_lock lock{ g_lowLevelKeyboardHookMutex };
            if (g_lowLevelKeyboardCallback)
            {
                auto callbackResult{ g_lowLevelKeyboardCallback(
                    wParam,
                    reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)
                ) };
                callNextHook = (callbackResult ==
                    FlashCom::Input::LowLevelCallbackReturnKind::Unhandled);
            }
        }
        if (callNextHook)
        {
            return ::CallNextHookEx(0, nCode, wParam, lParam);
        }
        return 1;
    }
}

namespace FlashCom::Input
{
    void SetGlobalLowLevelKeyboardCallback(
        std::function<LowLevelCallbackReturnKind(WPARAM, KBDLLHOOKSTRUCT*)> callback)
    {
        std::unique_lock lock{ g_lowLevelKeyboardHookMutex };
        if (g_hHook)
        {
            winrt::check_bool(::UnhookWindowsHookEx(g_hHook));
            g_hHook = 0;
        }
        g_lowLevelKeyboardCallback = callback;
        ::SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    }
}