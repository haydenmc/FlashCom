#pragma once
#include <cstdint>
#include <memory>
#include "Models/DataModel.h"

namespace LaunchTree
{
    struct App
    {
        App() = delete;
        static std::unique_ptr<App> CreateApp();
        void Show();
        void Hide();
        void KeyboardKeyDown(uint8_t vkey);

    private:
        App(std::unique_ptr<Models::DataModel>&& dataModel);
        std::unique_ptr<Models::DataModel> const m_dataModel;
    };
}