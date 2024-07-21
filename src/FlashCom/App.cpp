#include <pch.h>

#include "App.h"

#include <vector>
#include <unordered_set>

namespace
{
    const std::unordered_set<uint32_t> c_hotkeyCombo{ VK_LWIN, VK_SPACE };
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
        }
        else
        {
            m_hotkeysPressed.erase(kb->vkCode);
        }

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

            return FlashCom::Input::LowLevelCallbackReturnKind::Handled;
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

    void App::OnKeyDown(uint8_t vkeyCode)
    {
        for (const auto& childNode : m_dataModel->CurrentNode->GetChildren())
        {
            if (childNode->GetVkCode() == vkeyCode)
            {
                if (childNode->GetChildren().size() > 0)
                {
                    SPDLOG_INFO("App::OnKeyDown - Navigating to {}", childNode->GetName());
                    m_dataModel->CurrentNode = childNode;
                    m_ui.Update();
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
    {

    }

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
        m_ui.Update();
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
        m_ui.Update();
    }

    void App::OnExitCommand()
    {
        SPDLOG_INFO("App::OnExitCommand");
        PostMessageW(nullptr, WM_CLOSE, 0, 0);
    }
#pragma endregion Private
}