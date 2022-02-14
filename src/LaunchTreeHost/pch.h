#pragma once

// Windows headers
#include <windows.h>

// Workaround for GetCurrentTime definition collision
// See https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#how-do-i-resolve-ambiguities-with-getcurrenttime-and-or-try-
#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

// Windows SDK C++/WinRT
#include <Unknwn.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

// /Workaround
#pragma pop_macro("GetCurrentTime")

// Common STL headers
#include <string>
#include <string_view>

// App headers
#include <winrt/LaunchTreeApp.h>

// Namespace aliases
namespace winrt
{
    namespace WUI = Windows::UI;
    namespace WUIC = Windows::UI::Composition;
    namespace WUIXH = Windows::UI::Xaml::Hosting;
    namespace WF = Windows::Foundation;
}
