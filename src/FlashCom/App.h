#pragma once
#include "Models/DataModel.h"
#include "Settings/SettingsManager.h"
#include "View/HostWindow.h"
#include "View/TrayIcon.h"
#include "View/Ui.h"

#include <cstdint>
#include <memory>
#include <unordered_set>

namespace FlashCom
{
    struct App
    {
        static std::unique_ptr<App> CreateApp(const HINSTANCE& hInstance);
        int RunMessageLoop();
        void HandleLowLevelKeyboardInput(WPARAM wParam, KBDLLHOOKSTRUCT* kb);
        void ToggleVisibility();

    private:
        Settings::SettingsManager m_settingsManager;
        std::unique_ptr<Models::DataModel> const m_dataModel;
        View::HostWindow m_hostWindow;
        View::TrayIcon m_trayIcon;
        View::Ui m_ui;
        std::unordered_set<uint32_t> m_hotkeysPressed;
        bool m_isShowing{ false };

        App(const HINSTANCE& hInstance);
        void HandleFocusLost();
        void OnKeyDown(uint8_t vkeyCode);
        void OnKeyUp(uint8_t vkeyCode);
        void Show();
        void Hide();
        void OnSettingsCommand();
        void OnExitCommand();
    };
}