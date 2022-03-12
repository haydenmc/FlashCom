#include <pch.h>
#include "SettingsManager.h"

#include <string_view>
#include <winrt/Windows.Storage.h>

namespace winrt
{
    namespace WS = Windows::Storage;
}

namespace
{
    constexpr std::wstring_view c_settingsFileName{ L"settings.json" };
}

namespace LaunchTree::Settings
{

}
