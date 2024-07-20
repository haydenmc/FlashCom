#pragma once
#include <functional>
#include <string>
#include <utility>

namespace FlashCom::View
{
    using TrayIconMenuItem = std::pair<std::wstring, std::function<void()>>;
struct TrayIcon
{
    TrayIcon(const HINSTANCE& hInstance, const std::vector<TrayIconMenuItem>& menuItems);
    ~TrayIcon();

private:
    HINSTANCE const m_hInstance;
    ATOM const m_windowClass;
    HWND const m_hwnd;
    std::vector<TrayIconMenuItem> const m_menuItems;

    static LRESULT GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam,
        LPARAM lParam);
    LRESULT InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam,
        LPARAM lParam);
    void HandleTrayIconMessage(HWND hWnd, uint16_t param);
    void HandleTrayIconCommand(uint16_t param);
};
}
