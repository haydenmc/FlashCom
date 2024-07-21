#include "pch.h"
#include "TextVisual.h"

#include <winrt/Microsoft.Graphics.Canvas.Geometry.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Composition.h>
#include <winrt/Windows.Graphics.DirectX.h>

namespace winrt
{
    namespace MGC = Microsoft::Graphics::Canvas;
    namespace MGCG = Microsoft::Graphics::Canvas::Geometry;
    namespace MGCT = Microsoft::Graphics::Canvas::Text;
    namespace MGCUC = Microsoft::Graphics::Canvas::UI::Composition;
    namespace WGDX = Windows::Graphics::DirectX;
    namespace WUI = Windows::UI;
    namespace WUIC = Windows::UI::Composition;
}

namespace
{
    winrt::WUIC::SpriteVisual CreateTextVisual(
        winrt::WUIC::Compositor compositor,
        winrt::MGC::CanvasDevice canvasDevice,
        winrt::WUIC::CompositionGraphicsDevice graphicsDevice,
        winrt::MGCT::CanvasTextFormat textFormat,
        std::string_view content
    )
    {
        // Compute bounds of text
        winrt::MGCT::CanvasTextLayout textLayout{
            canvasDevice,
            winrt::to_hstring(content),
            textFormat,
            0,
            0
        };
        textLayout.WordWrapping(winrt::MGCT::CanvasWordWrapping::NoWrap);
        auto textBounds{ textLayout.LayoutBounds() };

        // Draw text
        auto drawingSurface{
            graphicsDevice.CreateDrawingSurface(
                { textBounds.Width, textBounds.Height },
                winrt::WGDX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                winrt::WGDX::DirectXAlphaMode::Premultiplied
            )
        };
        auto drawingSession{
            winrt::MGCUC::CanvasComposition::CreateDrawingSession(drawingSurface)
        };
        drawingSession.Clear(winrt::WUI::Colors::Transparent());
        drawingSession.DrawTextLayout(
            textLayout,
            { 0, 0 },
            winrt::WUI::Color{ 255, 192, 192, 192 }
        );
        auto drawingBrush{ compositor.CreateSurfaceBrush(drawingSurface) };

        // Try a blend mode
        winrt::MGCE::BlendEffect blendEffect{};
        blendEffect.Name(L"Blend");
        blendEffect.Mode(winrt::MGCE::BlendEffectMode::ColorDodge);
        blendEffect.Background(winrt::WUIC::CompositionEffectSourceParameter{ L"backdrop" });
        blendEffect.Foreground(winrt::WUIC::CompositionEffectSourceParameter{ L"text" });
        winrt::WUIC::CompositionEffectFactory blendEffectFactory{
            compositor.CreateEffectFactory(blendEffect) };
        winrt::WUIC::CompositionEffectBrush blendBrush{
            blendEffectFactory.CreateBrush() };

        auto backdropBrush{ compositor.CreateBackdropBrush() };
        blendBrush.SetSourceParameter(L"backdrop", backdropBrush);
        blendBrush.SetSourceParameter(L"text", drawingBrush);

        // Create composition visual
        auto spriteVisual{ compositor.CreateSpriteVisual() };
        spriteVisual.Brush(blendBrush);
        spriteVisual.Size(drawingSurface.Size());

        return spriteVisual;
    }
}

namespace FlashCom::View
{
    TextVisual::TextVisual(
        winrt::WUIC::Compositor compositor,
        winrt::MGC::CanvasDevice canvasDevice,
        winrt::WUIC::CompositionGraphicsDevice graphicsDevice,
        winrt::MGCT::CanvasTextFormat textFormat,
        std::string_view content
    ) :
        m_visual{
            CreateTextVisual(
                compositor,
                canvasDevice,
                graphicsDevice,
                textFormat,
                content
            )
        }
    { }

    winrt::WUIC::Visual TextVisual::Visual()
    {
        return m_visual;
    }
}