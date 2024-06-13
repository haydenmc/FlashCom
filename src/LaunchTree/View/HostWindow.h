#pragma once
#include <wtypes.h>

namespace LaunchTree::View
{
    struct HostWindow
    {
        HostWindow(const HINSTANCE& hInstance, std::wstring_view name);
        ~HostWindow();
        static LRESULT GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam,
            LPARAM lParam);
        LRESULT InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam,
            LPARAM lParam);
        HWND const GetHWnd() const;
        void MoveToCursorMonitor();
        void ShowAndForceToForeground();
        void Hide();
    private:
        std::wstring const m_name;
        HINSTANCE const m_hInstance;
        ATOM const m_windowClass;
        HWND const m_hwnd;
    };
}