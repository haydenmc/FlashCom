#include <pch.h>

#include "CompositionHost.h"

#include <DispatcherQueue.h>
#include <windows.ui.composition.interop.h>

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Effects.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Composition.h>
#include <winrt/Windows.Graphics.Effects.h>
#include <winrt/Windows.UI.Text.h>

namespace winrt
{
    namespace MGC = Microsoft::Graphics::Canvas;
    namespace MGCE = Microsoft::Graphics::Canvas::Effects;
    namespace MGCUC = Microsoft::Graphics::Canvas::UI::Composition;
    namespace WGDX = Windows::Graphics::DirectX;
    namespace WS = Windows::System;
    namespace WUIC = Windows::UI::Composition;
    namespace WUICD = Windows::UI::Composition::Desktop;
}

namespace
{
#pragma region Free functions
    inline winrt::WS::DispatcherQueueController CreateDispatcherQueue()
    {
        DispatcherQueueOptions options
        {
            .dwSize = sizeof(DispatcherQueueOptions),
            .threadType = DQTYPE_THREAD_CURRENT,
            .apartmentType = DQTAT_COM_ASTA,
        };

        winrt::WS::DispatcherQueueController controller{ nullptr };
        winrt::check_hresult(CreateDispatcherQueueController(
            options,
            reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(
                winrt::put_abi(controller))
        ));

        return controller;
    }

    inline winrt::WUICD::DesktopWindowTarget CreateDesktopWindowTarget(
        HWND window,
        winrt::WUIC::Compositor compositor
    )
    {
        namespace abi = ABI::Windows::UI::Composition::Desktop;

        auto interop = compositor
            .as<abi::ICompositorDesktopInterop>();
        winrt::WUICD::DesktopWindowTarget target{ nullptr };
        winrt::check_hresult(interop->CreateDesktopWindowTarget(
            window,
            false,
            reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))
        ));
        return target;
    }
#pragma endregion Free functions
}

namespace Hud
{
#pragma region Public
    CompositionHost::CompositionHost(HWND hWnd) :
        m_dispatcherQueueController{ CreateDispatcherQueue() },
        m_compositor{ winrt::WUIC::Compositor{} },
        m_target{ CreateDesktopWindowTarget(hWnd, m_compositor) }
    {
        auto root{ m_compositor.CreateContainerVisual() };
        root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        root.Offset({ 0, 0, 0 });
        m_target.Root(root);

        // Create some test visuals
        if (m_target.Root())
        {
            auto visuals = m_target.Root().as<winrt::WUIC::ContainerVisual>().Children();

            auto tl{ m_compositor.CreateSpriteVisual() };
            tl.Brush(m_compositor.CreateColorBrush({ 128, 255, 0, 0 }));
            tl.Size({ 50, 50 });
            tl.AnchorPoint({ 0, 0 });
            tl.RelativeOffsetAdjustment({ 0, 0, 0 });
            visuals.InsertAtTop(tl);

            auto tr{ m_compositor.CreateSpriteVisual() };
            tr.Brush(m_compositor.CreateColorBrush({ 128, 0, 255, 0 }));
            tr.Size({ 50, 50 });
            tr.AnchorPoint({ 1, 0 });
            tr.RelativeOffsetAdjustment({ 1, 0, 0 });
            visuals.InsertAtTop(tr);

            auto bl{ m_compositor.CreateSpriteVisual() };
            bl.Brush(m_compositor.CreateColorBrush({ 128, 0, 0, 255 }));
            bl.Size({ 50, 50 });
            bl.AnchorPoint({ 0, 1 });
            bl.RelativeOffsetAdjustment({ 0, 1, 0 });
            visuals.InsertAtTop(bl);

            auto br{ m_compositor.CreateSpriteVisual() };
            br.Brush(m_compositor.CreateColorBrush({ 128, 128, 0, 128 }));
            br.Size({ 50, 50 });
            br.AnchorPoint({ 1, 1 });
            br.RelativeOffsetAdjustment({ 1, 1, 0 });
            visuals.InsertAtTop(br);

            // Try to do some gaussian blur
            winrt::MGCE::GaussianBlurEffect blurEffect{};
            blurEffect.Name(L"Blur");
            blurEffect.BlurAmount(5.0f);
            blurEffect.BorderMode(winrt::MGCE::EffectBorderMode::Hard);
            blurEffect.Optimization(winrt::MGCE::EffectOptimization::Quality);
            blurEffect.Source(winrt::WUIC::CompositionEffectSourceParameter{ L"source" });
            winrt::WUIC::CompositionEffectFactory blurEffectFactory{
                m_compositor.CreateEffectFactory(blurEffect)
            };
            winrt::WUIC::CompositionEffectBrush blurBrush{
                blurEffectFactory.CreateBrush()
            };

            auto backdropBrush{ m_compositor.CreateBackdropBrush() };
            blurBrush.SetSourceParameter(L"source", backdropBrush);

            auto blurVisual{ m_compositor.CreateSpriteVisual() };
            blurVisual.Brush(blurBrush);
            blurVisual.Size({ 500, 500 });
            blurVisual.AnchorPoint({ 0.5, 0.5 });
            blurVisual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });
            visuals.InsertAtTop(blurVisual);

            // Try to render some text with win2d
            auto canvasDevice{ winrt::MGC::CanvasDevice::GetSharedDevice() };
            auto compositionGraphicsDevice{
                winrt::MGCUC::CanvasComposition::CreateCompositionGraphicsDevice(
                    m_compositor,
                    canvasDevice
                )
            };
            auto drawingSurface{
                compositionGraphicsDevice.CreateDrawingSurface(
                    { 512, 256 },
                    winrt::WGDX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                    winrt::WGDX::DirectXAlphaMode::Premultiplied
                )
            };

            auto win2dVisual{ m_compositor.CreateSpriteVisual() };
            win2dVisual.Brush(m_compositor.CreateSurfaceBrush(drawingSurface));
            win2dVisual.Size(drawingSurface.Size());
            win2dVisual.AnchorPoint({ 0.5, 0.5 });
            win2dVisual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });
            visuals.InsertAtTop(win2dVisual);

            {
                auto drawingSession{
                    winrt::MGCUC::CanvasComposition::CreateDrawingSession(drawingSurface)
                };
                drawingSession.Clear(winrt::Windows::UI::Colors::Transparent());

                winrt::MGC::Text::CanvasTextFormat textFormat{};
                textFormat.FontFamily(L"Segoe UI");
                textFormat.FontWeight(winrt::Windows::UI::Text::FontWeights::Black());
                textFormat.FontSize(64);
                textFormat.HorizontalAlignment(winrt::MGC::Text::CanvasHorizontalAlignment::Center);
                textFormat.VerticalAlignment(winrt::MGC::Text::CanvasVerticalAlignment::Center);

                drawingSession.DrawTextW(
                    L"HELLO WORLD",
                    { 256, 128 },
                    { 128, 255, 255, 255 },
                    textFormat
                );
            }
        }
    }
#pragma endregion Public

#pragma region Private
#pragma endregion Private
}