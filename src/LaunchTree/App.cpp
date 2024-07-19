#include <pch.h>

#include "App.h"

#include <vector>
#include <unordered_set>

namespace
{
    const std::unordered_set<uint32_t> c_hotkeyCombo{ VK_LWIN, VK_SPACE };

    std::unique_ptr<LaunchTree::Models::DataModel> CreateDataModel(
        LaunchTree::Settings::SettingsManager& settingsManager)
    {

        auto rootNode{ settingsManager.GetTree() };

        return std::make_unique<LaunchTree::Models::DataModel>(
            std::move(rootNode));
    }
}

namespace LaunchTree
{
#pragma region Public
    std::unique_ptr<App> App::CreateApp(const HINSTANCE& hInstance)
    {
        return std::unique_ptr<App>(new App(hInstance));
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

    void App::HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb)
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
            m_hotkeysPressed.insert(kb->vkCode);
        }
        else
        {
            m_hotkeysPressed.erase(kb->vkCode);
        }

        if (m_hotkeysPressed.size() == c_hotkeyCombo.size())
        {
            ToggleVisibility();
        }
    }

    void App::ToggleVisibility()
    {
        ::OutputDebugStringW(L"App::ToggleVisibility\n");
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
        m_dataModel{ std::move(CreateDataModel(m_settingsManager)) },
        m_hostWindow{ hInstance, L"LaunchTree", std::bind(&App::HandleFocusLost, this) },
        m_trayIcon{ hInstance, {
            std::make_pair(std::wstring{ L"Settings" }, std::bind(&App::OnSettingsCommand, this)),
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
                    m_dataModel->CurrentNode = childNode;
                    m_ui.Update();
                }
                else
                {
                    childNode->Execute();
                    Hide();
                }
            }
        }
    }

    void App::OnKeyUp(uint8_t /*vkeyCode*/)
    {

    }

    void App::Show()
    {
        m_isShowing = true;
        m_ui.Show();
    }

    void App::Hide()
    {
        m_isShowing = false;
        m_ui.Hide();
        m_dataModel->CurrentNode = m_dataModel->RootNode.get();
        m_ui.Update();
    }

    void App::OnSettingsCommand()
    {
        OutputDebugStringW(L"Settings invoked");
        ShellExecuteW(nullptr, L"explore",
            m_settingsManager.GetSettingsFilePath().parent_path().wstring().data(),
            nullptr, nullptr, SW_SHOWNORMAL);
    }

    void App::OnExitCommand()
    {
        PostMessageW(nullptr, WM_CLOSE, 0, 0);
    }
#pragma endregion Private
}