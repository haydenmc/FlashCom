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
        std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>> commsChildren;
        commsChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'W',
            L"WarmItUp",
            std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
        ));
        commsChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'V',
            L"Mumble",
            std::vector<std::unique_ptr<LaunchTree::Models::TreeNode>>{}
        ));
        rootChildren.push_back(std::make_unique<LaunchTree::Models::TreeNode>(
            'C',
            L"Comms",
            std::move(commsChildren)));

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
            switch (msg.message)
            {
            case WM_KEYDOWN:
                OnKeyDown(static_cast<uint8_t>(msg.wParam));
                break;
            case WM_KEYUP:
                OnKeyUp(static_cast<uint8_t>(msg.wParam));
                break;
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
            m_isShowing = false;
            m_ui.Hide();
            m_dataModel->CurrentNode = m_dataModel->RootNode.get();
            m_ui.Update();
        }
        else
        {
            m_isShowing = true;
            m_ui.Show();
        }
    }
#pragma endregion Public
#pragma region Private
    App::App(const HINSTANCE& hInstance) :
        m_dataModel{ std::move(CreateDataModel()) },
        m_hostWindow{ hInstance, L"LaunchTree", std::bind(&App::HandleFocusLost, this) },
        m_ui{m_hostWindow, m_dataModel.get()}
    { }

    void App::HandleFocusLost()
    {
        if (m_isShowing)
        {
            m_isShowing = false;
            m_ui.Hide();
        }
    }

    void App::OnKeyDown(uint8_t vkeyCode)
    {
        for (const auto& childNode : m_dataModel->CurrentNode->GetChildren())
        {
            if ((childNode->GetVkCode() == vkeyCode) &&
                (childNode->GetChildren().size() > 0))
            {
                m_dataModel->CurrentNode = childNode;
                m_ui.Update();
            }
        }
    }

    void App::OnKeyUp(uint8_t /*vkeyCode*/)
    {

    }
#pragma endregion Private
}