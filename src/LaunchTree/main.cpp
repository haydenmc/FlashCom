#include <pch.h>

#include "Input/LowLevelKeyboardHookHelper.h"
#include "Models/TreeNode.h"
#include "View/CompositionHost.h"

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr std::wstring_view c_windowTitle{ L"LaunchTree" };
    constexpr std::wstring_view c_windowClass{ L"LaunchTreeWindowClass" };
    HINSTANCE g_hInstance{ nullptr };
    std::unique_ptr<LaunchTree::View::CompositionHost> g_compositionHost{ nullptr };
    HWND g_hWnd{ nullptr };
    const std::unordered_set<uint32_t> c_hotkeyCombo{ VK_LWIN, VK_SPACE };
    std::unordered_set<uint32_t> g_hotkeysPressed;
    std::unordered_set<uint32_t> g_keysPressed;
    std::unique_ptr<LaunchTree::Models::TreeNode> g_rootNote{ nullptr };
}

// Forward declarations
int Run(HINSTANCE hInstance);
void HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb);
void OnHotkeyPress();
void HandleKeyboardMessage(UINT message, WPARAM vkCode);
void InitializeDataModel();
ATOM RegisterWindowClass(HINSTANCE hInstance);
void InitializeWindow(HINSTANCE hInstance);
void AdjustWindowSize(HWND hWnd);
void SetWindowStyles(HWND hWnd);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Console entrypoint
int main(
    int /*argc*/,
    wchar_t* /*argv*/[]
)
{
    return Run(GetModuleHandleW(nullptr));
}

// Windows entrypoint
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPWSTR /*lpCmdLine*/,
    int /*nCmdShow*/
)
{
    return Run(hInstance);
}

// Initializes window and runs the message loop
int Run(HINSTANCE hInstance)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    InitializeDataModel();

    RegisterWindowClass(hInstance);
    InitializeWindow(hInstance);

    LaunchTree::Input::SetGlobalLowLevelKeyboardCallback(HandleLowLevelKeyboardInput);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb)
{
    // Ignore non-hotkey keys
    if (c_hotkeyCombo.count(kb->vkCode) == 0)
    {
        return;
    }

    bool isKeyDown{ false };
    switch (wParam)
    {
    case WM_SYSKEYDOWN:
        __fallthrough;
    case WM_KEYDOWN:
        isKeyDown = true;
        break;
    case WM_SYSKEYUP:
        __fallthrough;
    case WM_KEYUP:
        isKeyDown = false;
        break;
    default:
        return;
    }

    if (isKeyDown)
    {
        g_hotkeysPressed.insert(kb->vkCode);
    }
    else
    {
        g_hotkeysPressed.erase(kb->vkCode);
    }

    if (g_hotkeysPressed.size() == c_hotkeyCombo.size())
    {
        OnHotkeyPress();
    }
}

void ForceForegroundWindow(HWND hwnd) {
    DWORD windowThreadProcessId = GetWindowThreadProcessId(GetForegroundWindow(), LPDWORD(0));
    DWORD currentThreadId = GetCurrentThreadId();
    AttachThreadInput(windowThreadProcessId, currentThreadId, true);
    BringWindowToTop(hwnd);
    ShowWindow(hwnd, SW_SHOW);
    AttachThreadInput(windowThreadProcessId, currentThreadId, false);
}

void OnHotkeyPress()
{
    if (!g_hWnd)
    {
        return;
    }

    if (IsWindowVisible(g_hWnd))
    {
        ShowWindow(g_hWnd, SW_HIDE);
    }
    else
    {
        AdjustWindowSize(g_hWnd);
        ShowWindow(g_hWnd, SW_SHOW);
        ForceForegroundWindow(g_hWnd);
    }
}

void HandleKeyboardMessage(UINT message, WPARAM wParam)
{
    auto vkCode{ static_cast<uint32_t>(wParam) };

    if (message == WM_KEYDOWN)
    {
        if (g_keysPressed.count(vkCode) == 0)
        {
            g_keysPressed.insert(vkCode);
        }
    }
    else if (message == WM_KEYUP)
    {
        g_keysPressed.erase(vkCode);
    }
}

void InitializeDataModel()
{
    std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>> rootChildren;
    rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
        'C',
        L"Comms",
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
    ));
    rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
        'D',
        L"Dev",
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
    ));
    rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
        'G',
        L"Gaming",
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
    ));

    auto rootNode{
        std::make_unique<LaunchTree::Models::TreeNode>(
            '\0',
            L"",
            std::move(rootChildren)
        )
    };

    g_rootNote = std::move(rootNode);
}

ATOM RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASSW windowClass
    {
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = nullptr,
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = nullptr,
        .lpszMenuName = nullptr,
        .lpszClassName = c_windowClass.data(),
    };

    return RegisterClass(&windowClass);
}

void InitializeWindow(HINSTANCE hInstance)
{
    g_hInstance = hInstance;

    g_hWnd = CreateWindowW(
        c_windowClass.data(),
        c_windowTitle.data(),
        WS_POPUP,
        0,
        0,
        400,
        800,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!g_hWnd)
    {
        throw std::runtime_error("Could not create window!");
    }

    SetWindowStyles(g_hWnd);
    AdjustWindowSize(g_hWnd);
    ShowWindow(g_hWnd, SW_SHOW);
    SetWindowStyles(g_hWnd);
    AdjustWindowSize(g_hWnd);

    g_compositionHost = std::make_unique<LaunchTree::View::CompositionHost>(g_hWnd);
}

void AdjustWindowSize(HWND hWnd)
{
    // Move the window to the active monitor
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
    
    // Set window to size of monitor, always on top
    SetWindowPos(
        hWnd,
        HWND_TOPMOST,
        monitorInfo.rcMonitor.left,
        monitorInfo.rcMonitor.top,
        monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
        monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
        SWP_SHOWWINDOW
    );
}

void SetWindowStyles(HWND hWnd)
{
    LONG extendedStyles{ GetWindowLongW(hWnd, GWL_EXSTYLE) };
    LONG newStyles
    {
        extendedStyles |
        WS_EX_LAYERED |    // https://docs.microsoft.com/en-us/windows/win32/winmsg/window-features
        WS_EX_TOOLWINDOW | // Don't show in taskbar/switcher
        WS_EX_TOPMOST |    // Always on top
        WS_EX_TRANSPARENT  // Make layered window clickthrough
    };
    LONG result{ SetWindowLongW(hWnd, GWL_EXSTYLE, newStyles) };
    result;
}

LRESULT CALLBACK WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_KEYDOWN:
        __fallthrough;
    case WM_KEYUP:
        HandleKeyboardMessage(message, wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            ::OutputDebugStringW(L"Focus lost.");
            if (g_hWnd && IsWindowVisible(g_hWnd))
            {
                ShowWindow(g_hWnd, SW_HIDE);
            }
        }
        __fallthrough;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
