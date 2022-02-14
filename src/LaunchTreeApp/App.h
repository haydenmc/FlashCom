#pragma once
#include "App.g.h"
#include "App.base.h"

namespace winrt::LaunchTreeApp::implementation
{
    class App : public AppT2<App>
    {
    public:
        App();
        ~App();
    };
}

namespace winrt::LaunchTreeApp::factory_implementation
{
    class App : public AppT<App, implementation::App>
    {
    };
}
