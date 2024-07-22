#pragma once
#include "HostWindow.h"

#include <vector>

namespace FlashCom::View
{
    struct BrushWithBounds
    {
        winrt::WF::Size Bounds;
        winrt::WUIC::CompositionSurfaceBrush Brush;
    };

    class CompositionManager
    {
    public:
        CompositionManager(HostWindow const & hostWindow);
        winrt::Windows::UI::Composition::Compositor GetCompositor();
        void PresentRootVisual(
            winrt::Windows::UI::Composition::ContainerVisual rootVisual);
        BrushWithBounds CreateTextBrush(winrt::MGCT::CanvasTextFormat textFormat,
            std::string_view content);

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
    };
}
