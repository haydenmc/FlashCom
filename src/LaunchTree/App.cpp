#include <pch.h>

#include "App.h"

#include <vector>
#include <unordered_set>

namespace
{
    const std::unordered_set<uint32_t> c_hotkeyCombo{ VK_LWIN, VK_SPACE };

    std::unique_ptr<LaunchTree::Models::DataModel> CreateDataModel()
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
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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
        if (m_isShowing)
        {
            m_ui.Hide();
            m_isShowing = false;
        }
        else
        {
            // TODO: Reset datamodel state
            m_ui.Update();
            m_ui.Show();
            m_isShowing = true;
        }
    }
#pragma endregion Public
#pragma region Private
    App::App(const HINSTANCE& hInstance) : m_dataModel{ std::move(CreateDataModel()) },
        m_hostWindow{ hInstance, L"LaunchTree" }, m_ui{ m_hostWindow, m_dataModel.get() }
    { }
#pragma endregion Private
}