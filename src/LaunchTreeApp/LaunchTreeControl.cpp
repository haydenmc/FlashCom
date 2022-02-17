#include "pch.h"
#include "LaunchTreeControl.h"
#if __has_include("LaunchTreeControl.g.cpp")
#include "LaunchTreeControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::LaunchTreeApp::implementation
{
    LaunchTreeControl::LaunchTreeControl()
    {
        InitializeComponent();
    }
}
