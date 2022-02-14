#include "pch.h"

namespace
{
    constexpr std::wstring_view c_windowTitle{ L"LaunchTree" };
    constexpr std::wstring_view c_windowClass{ L"LaunchTreeWindowClass" };
    HINSTANCE g_hInstance;
    winrt::LaunchTreeApp::App g_hostApp{ nullptr };
    winrt::WUIXH::DesktopWindowXamlSource g_desktopWindowXamlSource{ nullptr };
    winrt::LaunchTreeApp::LaunchTreeControl g_launchTreeControl{ nullptr };
}

// Forward declarations
ATOM RegisterWindowClass(HINSTANCE hInstance);
bool InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void AdjustLayout(HWND hWnd);

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow
)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_hostApp = winrt::LaunchTreeApp::App{};
    g_desktopWindowXamlSource = winrt::WUIXH::DesktopWindowXamlSource{};

    RegisterWindowClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return false;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM RegisterWindowClass(
    HINSTANCE hInstance
)
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
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszMenuName = nullptr,
        .lpszClassName = c_windowClass.data(),
    };

    return RegisterClass(&windowClass);
}

bool InitInstance(
    HINSTANCE hInstance,
    int nCmdShow
)
{
    g_hInstance = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(
        c_windowClass.data(),
        c_windowTitle.data(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        return false;
    }

    // XAML islands hosting
    if (g_desktopWindowXamlSource)
    {
        auto interop{ g_desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>() };
        winrt::check_hresult(interop->AttachToWindow(hWnd));
        HWND hWndXamlIsland{ nullptr };
        winrt::check_hresult(interop->get_WindowHandle(&hWndXamlIsland)); \
            RECT windowRect;
        ::GetWindowRect(hWnd, &windowRect);
        ::SetWindowPos(
            hWndXamlIsland,
            nullptr,
            0,
            0,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            SWP_SHOWWINDOW
        );
        g_launchTreeControl = winrt::LaunchTreeApp::LaunchTreeControl{};
        g_desktopWindowXamlSource.Content(g_launchTreeControl);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return true;
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
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        if (g_desktopWindowXamlSource)
        {
            g_desktopWindowXamlSource.Close();
            g_desktopWindowXamlSource = nullptr;
        }
        break;
    case WM_SIZE:
        AdjustLayout(hWnd);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void AdjustLayout(HWND hWnd)
{
    if (g_desktopWindowXamlSource)
    {
        auto interop{ g_desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>() };
        HWND xamlHostHwnd{ nullptr };
        winrt::check_hresult(interop->get_WindowHandle(&xamlHostHwnd));
        RECT windowRect{ 0 };
        ::GetWindowRect(hWnd, &windowRect);
        ::SetWindowPos(
            xamlHostHwnd,
            nullptr,
            0,
            0,
            (windowRect.right - windowRect.left),
            (windowRect.bottom - windowRect.top),
            SWP_SHOWWINDOW
        );
    }
}
