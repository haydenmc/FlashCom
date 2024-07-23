#include <pch.h>

#include "App.h"
#include "Input/LowLevelKeyboardHookHelper.h"

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

namespace
{
    constexpr std::string_view c_logFileName{ "FlashCom.log" };
    HINSTANCE g_hInstance{ nullptr };
    std::unique_ptr<FlashCom::App> g_app{ nullptr };

    FlashCom::Input::LowLevelCallbackReturnKind HandleLowLevelKeyboardInput(
        WPARAM wParam, KBDLLHOOKSTRUCT* kb)
    {
        if (g_app)
        {
            return g_app->HandleLowLevelKeyboardInput(wParam, kb);
        }
        else
        {
            SPDLOG_ERROR("HandleLowLevelKeyboardInput - Low level keyboard input unhandled, "
                "callback not set.");
            return FlashCom::Input::LowLevelCallbackReturnKind::Unhandled;
        }
    }

    int Run(HINSTANCE hInstance)
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        auto logFilePath{ FlashCom::Settings::GetApplicationLocalDataDirectory() / c_logFileName };
        InitializeLogging(logFilePath.string());
        g_app = FlashCom::App::CreateApp(hInstance);
        FlashCom::Input::SetGlobalLowLevelKeyboardCallback(HandleLowLevelKeyboardInput);
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
