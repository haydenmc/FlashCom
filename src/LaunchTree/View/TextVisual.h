#pragma once
#include "ICompositionVisual.h"

#include <string>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Windows.UI.Composition.h>

namespace LaunchTree::View
{
    class TextVisual : public ICompositionVisual
    {
    public:
        TextVisual(
            winrt::Windows::UI::Composition::Compositor compositor,
            winrt::Microsoft::Graphics::Canvas::CanvasDevice canvasDevice,
            winrt::Windows::UI::Composition::CompositionGraphicsDevice graphicsDevice,
            winrt::Microsoft::Graphics::Canvas::Text::CanvasTextFormat textFormat,
            std::wstring_view content
        );

        // ICompositionVisual
        virtual winrt::Windows::UI::Composition::Visual Visual() override;

    private:
        const winrt::Windows::UI::Composition::SpriteVisual m_visual{ nullptr };
    };
}
