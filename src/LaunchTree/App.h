#pragma once
#include "Models/DataModel.h"
#include "View/HostWindow.h"
#include "View/Ui.h"

#include <cstdint>
#include <memory>
#include <unordered_set>

namespace LaunchTree
{
    struct App
    {
        static std::unique_ptr<App> CreateApp(const HINSTANCE& hInstance);
        int RunMessageLoop();
        void HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb);
        void ToggleVisibility();

    private:
        App(const HINSTANCE& hInstance);
        std::unique_ptr<Models::DataModel> const m_dataModel;
        View::HostWindow m_hostWindow;
        View::Ui m_ui;
        std::unordered_set<uint32_t> m_hotkeysPressed;
        bool m_isShowing{ false };
    };
}