#include <pch.h>

#include "App.h"
#include "Input/LowLevelKeyboardHookHelper.h"

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr std::wstring_view c_windowTitle{ L"LaunchTree" };
    constexpr std::wstring_view c_windowClass{ L"LaunchTreeWindowClass" };
    HINSTANCE g_hInstance{ nullptr };
    std::unique_ptr<LaunchTree::App> g_app{ nullptr };

    void HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb)
    {
        if (g_app)
        {
            g_app->HandleLowLevelKeyboardInput(wParam, kb);
        }
        else
        {
            // TODO: Log error "low-level keyboard input not handled"
        }
    }

    int Run(HINSTANCE hInstance)
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        g_app = LaunchTree::App::CreateApp(hInstance);
        LaunchTree::Input::SetGlobalLowLevelKeyboardCallback(HandleLowLevelKeyboardInput);
        return g_app->RunMessageLoop();
    }
}

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
