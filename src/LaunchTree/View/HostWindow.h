#pragma once
#include <functional>

namespace LaunchTree::View
{
    struct HostWindow
    {
        HostWindow(const HINSTANCE& hInstance, std::wstring_view name,
            std::function<void()> onFocusLostCallback);
        ~HostWindow();
        static LRESULT GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam,
            LPARAM lParam);
        LRESULT InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam,
            LPARAM lParam);
        HWND const GetHWnd() const;
        void ShowAndForceToForeground();
        void Hide();
    private:
        HINSTANCE const m_hInstance;
        std::wstring const m_name;
        std::function<void()> m_onFocusLostCallback;
        ATOM const m_windowClass;
        HWND const m_hwnd;
    };
}