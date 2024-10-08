#include <pch.h>
#include "HostWindow.h"
#include <memory>
#include <string_view>

namespace
{
    constexpr std::wstring_view c_windowTitle{ L"FlashCom" };
    constexpr std::wstring_view c_windowClass{ L"FlashComWindowClass" };

    ATOM CreateWindowClass(const std::wstring& className, WNDPROC wndProc, HINSTANCE hInstance)
    {
        WNDCLASSW windowClass
        {
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = wndProc,
            .cbClsExtra = 0,
            // This is where we'll store a pointer to the HostWindow instance
            .cbWndExtra = sizeof(FlashCom::View::HostWindow*),
            .hInstance = hInstance,
            .hIcon = nullptr,
            .hCursor = LoadCursorW(nullptr, IDC_ARROW),
            .hbrBackground = nullptr,
            .lpszMenuName = nullptr,
            .lpszClassName = className.data(),
        };
        ATOM classRegistration{ RegisterClassW(&windowClass) };
        winrt::check_bool(classRegistration);
        return classRegistration;
    }

    HWND CreateWindowInstance(FlashCom::View::HostWindow* instancePointer,
        const std::wstring& className, const std::wstring& title,
        const HINSTANCE& hInstance)
    {
        HWND hWnd{ CreateWindowW(
            className.data(),
            title.data(),
            WS_POPUP,
            0,
            0,
            400,
            800,
            nullptr,
            nullptr,
            hInstance,
            nullptr
        ) };
        winrt::check_pointer(hWnd);

        LONG extendedStyles{ GetWindowLongW(hWnd, GWL_EXSTYLE) };
        LONG newStyles
        {
            // See https://docs.microsoft.com/en-us/windows/win32/winmsg/window-features/
            // and https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
            extendedStyles |
            WS_EX_TOOLWINDOW | // Don't show in taskbar/switcher
            WS_EX_TOPMOST      // Always on top
        };
        SetWindowLongW(hWnd, GWL_EXSTYLE, newStyles);

        SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(instancePointer));

        return hWnd;
    }
}

namespace FlashCom::View
{
#pragma region Public
HostWindow::HostWindow(const HINSTANCE& hInstance, std::wstring_view name,
    std::function<void()> onFocusLostCallback) :
    m_name{ name },
    m_hInstance{ hInstance },
    m_onFocusLostCallback{ onFocusLostCallback },
    m_windowClass{ CreateWindowClass(
        (std::wstring{ name } + L"WindowClass"),
        &HostWindow::GlobalWndProc,
        hInstance) },
    m_hwnd{ CreateWindowInstance(
        this,
        (std::wstring{ name } + L"WindowClass"),
        std::wstring{ name }, hInstance) }
{
    SPDLOG_INFO("HostWindow::HostWindow - Created");
}

HostWindow::~HostWindow()
{
    DestroyWindow(m_hwnd);
    UnregisterClassW((std::wstring{ m_name } + L"WindowClass").data(), m_hInstance);
}

LRESULT HostWindow::GlobalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Retrieve instance pointer
    HostWindow* pWnd = reinterpret_cast<HostWindow*>(GetWindowLongPtrW(hWnd, 0));
    if (pWnd != nullptr)
    {
        return pWnd->InstanceWndProc(hWnd, message, wParam, lParam);
    }

    // TODO: Log error
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT HostWindow::InstanceWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        __fallthrough;
    case WM_KEYUP:
        // TODO
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_ACTIVATE:
        if ((LOWORD(wParam) == WA_INACTIVE) &&
            (reinterpret_cast<HWND>(lParam) != m_hwnd))
        {
            SPDLOG_INFO("HostWindow::InstanceWndProc - WM_ACTIVATE activation lost.");
            if (m_onFocusLostCallback)
            {
                m_onFocusLostCallback();
            }
        }
        return 0;
    case WM_KILLFOCUS:
        if (reinterpret_cast<HWND>(wParam) != m_hwnd)
        {
            SPDLOG_INFO("HostWindow::InstanceWndProc - WM_KILLFOCUS focus lost.");
        }
        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

HWND const HostWindow::GetHWnd() const
{
    return m_hwnd;
}

HostWindow::WindowShowTicket HostWindow::PrepareToShow()
{
    // First, get the cursor so we can find which monitor is "active"
    CURSORINFO cursorInfo
    {
        .cbSize = sizeof(CURSORINFO)
    };
    winrt::check_bool(GetCursorInfo(&cursorInfo));

    // Retrieve the monitor at the current cursor coordinates
    HMONITOR monitor{ MonitorFromPoint(cursorInfo.ptScreenPos, MONITOR_DEFAULTTONEAREST) };
    MONITORINFO monitorInfo
    {
        .cbSize = sizeof(MONITORINFO)
    };
    winrt::check_bool(GetMonitorInfoW(monitor, &monitorInfo));

    SPDLOG_INFO("HostWindow::PrepareToShow - Preparing to show on monitor ({}, {}) {} x {}",
        monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
        (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
        (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top));

    // Return a ticket that the caller can use to actually show the window
    HWND showHwnd{ m_hwnd };
    std::function<void()> showFunction{
        [showHwnd, monitorInfo]() -> void
        {
            SPDLOG_INFO("HostWindow::PrepareToShow - Ticket called, showing on monitor "
                "({}, {}) {} x {}",
                monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
                (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top));

            // Workaround to make sure Windows lets us bring our hwnd to the top
            DWORD windowThreadProcessId = GetWindowThreadProcessId(
                GetForegroundWindow(), LPDWORD(0));
            DWORD currentThreadId = GetCurrentThreadId();
            AttachThreadInput(windowThreadProcessId, currentThreadId, true);

            // Set window to size of monitor, always on top
            winrt::check_bool(SetWindowPos(
                showHwnd,
                HWND_TOPMOST,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
                (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top),
                SWP_SHOWWINDOW
            ));

            // Set keyboard focus
            SetFocus(showHwnd);

            // /Workaround
            AttachThreadInput(windowThreadProcessId, currentThreadId, false);
        }
    };

    return HostWindow::WindowShowTicket{
        .WindowSize = {
            (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
            (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top)
        },
        .ShowWindow = showFunction
    };
}

void HostWindow::Hide()
{
    SPDLOG_INFO("HostWindow::Hide");
    ShowWindow(m_hwnd, SW_HIDE);
}
#pragma endregion Public
}