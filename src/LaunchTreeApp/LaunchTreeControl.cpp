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

    int32_t LaunchTreeControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void LaunchTreeControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void LaunchTreeControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
