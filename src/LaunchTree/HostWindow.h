#pragma once
#include <wtypes.h>

namespace LaunchTree
{
    struct HostWindow
    {
        HostWindow();
        ~HostWindow();
    private:
        HWND m_hwnd;
    };
}