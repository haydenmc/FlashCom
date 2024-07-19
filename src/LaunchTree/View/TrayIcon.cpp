#include <pch.h>
#include "TrayIcon.h"
#include <shellapi.h>

namespace
{
    constexpr std::wstring_view c_windowClass{ L"LaunchTreeTrayIconWindowClass" };
    constexpr uint32_t c_notificationTrayIconId{ 1 };
    constexpr uint32_t c_notificationTrayIconMessageId{ WM_USER + 0x100 };
    constexpr uint16_t c_menuItemIdOffset{ 0x100 };

    void AddTrayIcon(const HINSTANCE& hInstance, HWND hWnd)
    {
        HICON hIcon{ static_cast<HICON>(::LoadImage(hInstance,
            MAKEINTRESOURCEW(IDI_ICON1),
            IMAGE_ICON,
            128, 128,
            LR_DEFAULTCOLOR)) };

        NOTIFYICONDATA iconData{};
        iconData.cbSize = sizeof(iconData);
        iconData.hWnd = hWnd;
        iconData.uID = c_notificationTrayIconId;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        iconData.uCallbackMessage = c_notificationTrayIconMessageId;
        iconData.uVersion = NOTIFYICON_VERSION_4;
        iconData.hIcon = hIcon;

        if (Shell_NotifyIconW(NIM_ADD, &iconData))
        {
            Shell_NotifyIconW(NIM_SETVERSION, &iconData);
        }
    }

    void RemoveTrayIcon(HWND hWnd)
    {
        NOTIFYICONDATA iconData{};
        iconData.cbSize = sizeof(iconData);
        iconData.hWnd = hWnd;
        iconData.uID = c_notificationTrayIconId;

        Shell_NotifyIconW(NIM_DELETE, &iconData);
    }

    ATOM CreateWindowClass(const std::wstring& className, WNDPROC wndProc, HINSTANCE hInstance)
    {
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = wndProc;
        // This is where we'll store a pointer to the HostWindow instance
        windowClass.cbWndExtra = sizeof(LaunchTree::View::TrayIcon*);
        windowClass.hInstance = hInstance;
        windowClass.lpszClassName = className.data();
        ATOM classRegistration{ RegisterClassW(&windowClass) };
        winrt::check_bool(classRegistration);
        return classRegistration;
    }

    HWND CreateWindowInstance(LaunchTree::View::TrayIcon* instancePointer,
        const std::wstring& className, const HINSTANCE& hInstance)
    {
        HWND hWnd{ CreateWindowExW(
            0,
            className.data(),
            nullptr,
            0,
            0,
            0,
            0,
            0,
            HWND_MESSAGE,
            nullptr,
            hInstance,
            nullptr
        ) };
        winrt::check_pointer(hWnd);
        SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(instancePointer));
        return hWnd;
    }
}

namespace LaunchTree::View
{
    TrayIcon::TrayIcon(const HINSTANCE& hInstance,
        const std::vector<TrayIconMenuItem>& menuItems) : m_hInstance{ hInstance },
        m_windowClass{ CreateWindowClass(std::wstring{ c_windowClass },
            &TrayIcon::GlobalWndProc, hInstance) },
        m_hwnd{ CreateWindowInstance(this, std::wstring{ c_windowClass }, hInstance) },
        m_menuItems{ menuItems }
    {
        AddTrayIcon(hInstance, m_hwnd);
    }

    TrayIcon::~TrayIcon()
    {
        RemoveTrayIcon(m_hwnd);
        DestroyWindow(m_hwnd);
        UnregisterClassW(c_windowClass.data(), m_hInstance);
    }

    LRESULT TrayIcon::GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // Retrieve instance pointer
        TrayIcon* pWnd = reinterpret_cast<TrayIcon*>(GetWindowLongPtrW(hWnd, 0));
        if (pWnd != nullptr)
        {
            return pWnd->InstanceWndProc(hWnd, message, wParam, lParam);
        }

        // TODO: Log error
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    LRESULT TrayIcon::InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case c_notificationTrayIconMessageId:
            HandleTrayIconMessage(hWnd, LOWORD(lParam));
            return 0;
        case WM_COMMAND:
            if (lParam == 0)
            {
                HandleTrayIconCommand(LOWORD(wParam));
            }
            break;
        case WM_NCDESTROY:
            // TODO: Remove tray icon
            break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    void TrayIcon::HandleTrayIconMessage(HWND hWnd, uint16_t param)
    {
        switch (param)
        {
        case NIN_SELECT:
        case NIN_KEYSELECT:
        case WM_CONTEXTMENU:
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu{ CreatePopupMenu() };
            for (size_t i{ 0 }; i < m_menuItems.size(); ++i)
            {
                const auto& menuItem{ m_menuItems.at(i) };
                InsertMenuW(hMenu, static_cast<UINT>(-1), MF_BYPOSITION | MF_STRING,
                    (c_menuItemIdOffset + i), menuItem.first.data());
            }
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                pt.x, pt.y, 0, hWnd, nullptr);
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackpopupmenu#remarks
            PostMessageW(hWnd, WM_NULL, 0, 0);
        }
    }

    void TrayIcon::HandleTrayIconCommand(uint16_t param)
    {
        auto commandIndex{ param - c_menuItemIdOffset };
        if (commandIndex >= m_menuItems.size())
        {
            // TODO: Log error
            return;
        }
        const auto& menuItem{ m_menuItems.at(commandIndex) };
        if (menuItem.second)
        {
            menuItem.second();
        }
    }
}