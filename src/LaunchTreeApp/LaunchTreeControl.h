#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "LaunchTreeControl.g.h"

namespace winrt::LaunchTreeApp::implementation
{
    struct LaunchTreeControl : LaunchTreeControlT<LaunchTreeControl>
    {
        LaunchTreeControl();
    };
}

namespace winrt::LaunchTreeApp::factory_implementation
{
    struct LaunchTreeControl : LaunchTreeControlT<LaunchTreeControl, implementation::LaunchTreeControl>
    {
    };
}
