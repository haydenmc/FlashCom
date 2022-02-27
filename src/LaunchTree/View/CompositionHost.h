#pragma once
#include <windef.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>

namespace LaunchTree::View
{
    class CompositionHost
    {
    public:
        CompositionHost(HWND hWnd);

    private:
        winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController{ nullptr };
        winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
        winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_target{ nullptr };
    };
}
