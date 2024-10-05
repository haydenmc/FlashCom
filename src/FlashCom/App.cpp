#include <pch.h>

#include "App.h"

#include <vector>
#include <unordered_set>

namespace
{
    const std::unordered_set<uint32_t> c_hotkeyCombo{ VK_LWIN, VK_SPACE };
    constexpr std::chrono::milliseconds c_hotkeyExpirationTime{ 1000 };

    void ShowStartupNotification()
    {
        winrt::WDXD::XmlDocument notificationPayload;
        notificationPayload.LoadXml(LR""""(
<toast launch="-from-startup-toast">
    <visual>
        <binding template="ToastGeneric">
            <text>FlashCom is running!</text>
            <text>Press Win+Space to invoke.</text>
            <text>Manage settings from the system tray icon.</text>
        </binding>
    </visual>
</toast>
)"""");
        winrt::WUIN::ToastNotification notification{ notificationPayload };
        auto notificationManager{ winrt::WUIN::ToastNotificationManager::GetDefault() };
        auto notifier{ notificationManager.CreateToastNotifier() };
        notifier.Show(notification);
    }
}

namespace FlashCom
{
#pragma region Public
    std::unique_ptr<App> App::CreateApp(const HINSTANCE& hInstance)
    {
        std::unique_ptr<App> app{ new App(hInstance) };
        app->LoadDataModel();
        return app;
    }

    void App::LoadDataModel()
    {
        auto result{ m_settingsManager.LoadSettings() };
        m_dataModel->ShowStartupNotification = m_settingsManager.GetShowStartupNotification();
        m_dataModel->UseTwentyFourHourClock = m_settingsManager.UseTwentyFourHourClock();
        m_dataModel->RootNode = m_settingsManager.GetCommandTreeRoot();
        m_dataModel->CurrentNode = m_dataModel->RootNode.get();
        if (!result.has_value())
        {
            m_dataModel->LoadErrorMessage = result.error();
        }
        else
        {
            m_dataModel->LoadErrorMessage = "";
        }
    }

    int App::RunMessageLoop()
    {
        // On launch, notify the user that we are, in fact, running.
        // FlashCom has failed Windows Store certification in the past because
        // the tester thought the app didn't start successfully.
        if (m_dataModel->ShowStartupNotification)
        {
            SPDLOG_INFO("App::RunMessageLoop - Showing startup notification");
            ShowStartupNotification();
        }

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            switch (msg.message)
            {
            case WM_KEYDOWN:
                OnKeyDown(static_cast<uint8_t>(msg.wParam));
                break;
            case WM_KEYUP:
                OnKeyUp(static_cast<uint8_t>(msg.wParam));
                break;
            case WM_CLOSE:
                return 0;
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return (int)msg.wParam;
    }

    FlashCom::Input::LowLevelCallbackReturnKind App::HandleLowLevelKeyboardInput(
        WPARAM wParam, KBDLLHOOKSTRUCT* kb)
    {
        // Ignore non-hotkey keys
        if (c_hotkeyCombo.count(kb->vkCode) == 0)
        {
            return FlashCom::Input::LowLevelCallbackReturnKind::Unhandled;
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
            return FlashCom::Input::LowLevelCallbackReturnKind::Unhandled;
        }

        if (isKeyDown)
        {
            m_hotkeysPressed.insert(kb->vkCode);
            if (m_hotkeysPressed.size() == c_hotkeyCombo.size())
            {
                SPDLOG_INFO("App::HandleLowLevelKeyboardInput - Hotkey pressed");
                ToggleVisibility();

                // This dummy event is to prevent Start from thinking that the Win key was
                // just pressed.
                // See https://github.com/microsoft/PowerToys/pull/4874
                INPUT dummyEvent[1] = {};
                dummyEvent[0].type = INPUT_KEYBOARD;
                dummyEvent[0].ki.wVk = 0xFF;
                dummyEvent[0].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, dummyEvent, sizeof(INPUT));

                // Reset hotkey state to avoid any stuck keys or accidental invocations
                m_hotkeysPressed.clear();

                // Cancel reset timer
                if (m_hotkeyResetTimer)
                {
                    m_hotkeyResetTimer.Cancel();
                    m_hotkeyResetTimer = nullptr;
                }

                return FlashCom::Input::LowLevelCallbackReturnKind::Handled;
            }
            else
            {
                // Every time a new hotkey key is pressed, (re)set a timer.
                // Once the timer elapses, reset hotkey key state.
                // This is to handle situations where key events are "eaten" before they reach us
                // (ex. User invokes lock with Win+L, lock screen eats the key-up events)
                // Otherwise keys might get stuck in a "pressed" state.
                if (m_hotkeyResetTimer)
                {
                    m_hotkeyResetTimer.Cancel();
                }
                m_hotkeyResetTimer = winrt::WST::ThreadPoolTimer::CreateTimer(
                    { this, &App::OnHotkeyTimerElapsed }, c_hotkeyExpirationTime);
            }
        }
        else
        {
            m_hotkeysPressed.erase(kb->vkCode);
        }

        
        return FlashCom::Input::LowLevelCallbackReturnKind::Unhandled;
    }

    void App::ToggleVisibility()
    {
        SPDLOG_INFO("App::ToggleVisibility");
        if (m_isShowing)
        {
            Hide();
        }
        else
        {
            Show();
        }
    }
#pragma endregion Public
#pragma region Private
    App::App(const HINSTANCE& hInstance) :
        m_dataModel{ std::make_unique<FlashCom::Models::DataModel>() },
        m_hostWindow{ hInstance, L"FlashCom", std::bind(&App::HandleFocusLost, this) },
        m_trayIcon{ hInstance, {
            std::make_pair(std::wstring{ L"Settings" }, std::bind(&App::OnSettingsCommand, this)),
            std::make_pair(std::wstring{ L"Reload" }, std::bind(&App::OnReloadCommand, this)),
            std::make_pair(std::wstring{ L"Exit" }, std::bind(&App::OnExitCommand, this))
        } },
        m_ui{ m_hostWindow, m_dataModel.get() }
    { }

    void App::HandleFocusLost()
    {
        if (m_isShowing)
        {
            Hide();
        }
    }

    void App::OnHotkeyTimerElapsed(winrt::WST::ThreadPoolTimer timer)
    {
        // See comment in App::HandleLowLevelKeyboardInput
        SPDLOG_INFO("App::OnHotkeyTimerElapsed - Timer elapsed, clearing hotkey state");
        m_hotkeysPressed.clear();
        m_hotkeyResetTimer = nullptr;
    }

    void App::OnKeyDown(uint8_t vkeyCode)
    {
        if (vkeyCode == VK_ESCAPE)
        {
            SPDLOG_INFO("App::OnKeyDown - Escape key pressed; hiding");
            Hide();
            return;
        }
        for (const auto& childNode : m_dataModel->CurrentNode->GetChildren())
        {
            if (childNode->GetVkCode() == vkeyCode)
            {
                if (childNode->GetChildren().size() > 0)
                {
                    SPDLOG_INFO("App::OnKeyDown - Navigating to {}", childNode->GetName());
                    m_dataModel->CurrentNode = childNode;
                    m_ui.Update(View::UpdateReasonKind::Navigating);
                }
                else
                {
                    SPDLOG_INFO("App::OnKeyDown - Executing {}", childNode->GetName());
                    Hide();
                    childNode->Execute();
                }
            }
        }
    }

    void App::OnKeyUp(uint8_t /*vkeyCode*/)
    { }

    void App::Show()
    {
        SPDLOG_INFO("App::Show");
        m_isShowing = true;
        m_ui.Show();
    }

    void App::Hide()
    {
        SPDLOG_INFO("App::Hide");
        m_isShowing = false;
        m_ui.Hide();
        m_dataModel->CurrentNode = m_dataModel->RootNode.get();
        m_ui.Update(View::UpdateReasonKind::Hiding);
    }

    void App::OnSettingsCommand()
    {
        SPDLOG_INFO("App::OnSettingsCommand");
        ShellExecuteW(nullptr, L"explore",
            m_settingsManager.GetSettingsFilePath().parent_path().wstring().data(),
            nullptr, nullptr, SW_SHOWNORMAL);
    }

    void App::OnReloadCommand()
    {
        SPDLOG_INFO("App::OnReloadCommand");
        LoadDataModel();
        m_ui.Update(View::UpdateReasonKind::Reloading);
    }

    void App::OnExitCommand()
    {
        SPDLOG_INFO("App::OnExitCommand");
        PostMessageW(nullptr, WM_CLOSE, 0, 0);
    }
#pragma endregion Private
}