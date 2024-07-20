#include <pch.h>
#include "LowLevelKeyboardHookHelper.h"

#include <shared_mutex>

namespace
{
    std::shared_mutex g_lowLevelKeyboardHookMutex;
    ::HHOOK g_hHook{ 0 };
    std::function<void(WPARAM, KBDLLHOOKSTRUCT*)> g_lowLevelKeyboardCallback;

    LRESULT LowLevelKeyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        {
            std::shared_lock lock{ g_lowLevelKeyboardHookMutex };
            if (g_lowLevelKeyboardCallback)
            {
                g_lowLevelKeyboardCallback(
                    wParam,
                    reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)
                );
            }
        }

        return ::CallNextHookEx(0, nCode, wParam, lParam);
    }
}

namespace FlashCom::Input
{
    void SetGlobalLowLevelKeyboardCallback(std::function<void(WPARAM, KBDLLHOOKSTRUCT*)> callback)
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