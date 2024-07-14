#pragma once
#include "HostWindow.h"
#include "ICompositionVisual.h"

#include <vector>

namespace LaunchTree::View
{
    class CompositionHost
    {
    public:
        CompositionHost(HostWindow const & hostWindow);

        winrt::Windows::UI::Composition::ContainerVisual CreateRootVisual();
        void PresentRootVisual(
            winrt::Windows::UI::Composition::ContainerVisual rootVisual);

        template<class C, class... Args>
        std::unique_ptr<C> CreateVisual(Args... args)
        {
            return std::make_unique<C>(m_compositor, m_canvasDevice, m_graphicsDevice, args...);
        }

    private:
        HostWindow const & m_hostWindow;
        const winrt::Windows::System::DispatcherQueueController
            m_dispatcherQueueController{ nullptr };
        const winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
        const winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_target{ nullptr };
        // Win2D
        const winrt::Microsoft::Graphics::Canvas::CanvasDevice m_canvasDevice{ nullptr };
        const winrt::Windows::UI::Composition::CompositionGraphicsDevice
            m_graphicsDevice{ nullptr };

        std::vector<std::unique_ptr<ICompositionVisual>> m_visuals;
    };
}
