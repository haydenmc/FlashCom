#include <pch.h>

#include "CompositionHost.h"
#include "TextVisual.h"

#include <DispatcherQueue.h>


namespace
{
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

        auto interop{ compositor.as<abi::ICompositorDesktopInterop>() };
        winrt::WUICD::DesktopWindowTarget target{ nullptr };
        winrt::check_hresult(interop->CreateDesktopWindowTarget(
            window,
            false,
            reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))
        ));
        return target;
    }
}

namespace FlashCom::View
{
#pragma region Public
    CompositionHost::CompositionHost(HostWindow const& hostWindow) :
        m_hostWindow{ hostWindow },
        m_dispatcherQueueController{ CreateDispatcherQueue() },
        m_compositor{ winrt::WUIC::Compositor{} },
        m_target{ CreateDesktopWindowTarget(m_hostWindow.GetHWnd(), m_compositor)},
        m_canvasDevice{ winrt::MGC::CanvasDevice::GetSharedDevice() },
        // TODO: Handle RenderingDeviceReplaced
        m_graphicsDevice{
            winrt::MGCUC::CanvasComposition::CreateCompositionGraphicsDevice(
                    m_compositor,
                    m_canvasDevice
            )
        }
    {
        //auto root{ m_compositor.CreateContainerVisual() };
        //root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        //root.Offset({ 0, 0, 0 });
        //m_target.Root(root);

        //// Create some test visuals
        //if (m_target.Root())
        //{
        //    auto visuals = m_target.Root().as<winrt::WUIC::ContainerVisual>().Children();

        //    auto tl{ m_compositor.CreateSpriteVisual() };
        //    tl.Brush(m_compositor.CreateColorBrush({ 128, 255, 0, 0 }));
        //    tl.Size({ 50, 50 });
        //    tl.AnchorPoint({ 0, 0 });
        //    tl.RelativeOffsetAdjustment({ 0, 0, 0 });
        //    visuals.InsertAtTop(tl);

        //    auto tr{ m_compositor.CreateSpriteVisual() };
        //    tr.Brush(m_compositor.CreateColorBrush({ 128, 0, 255, 0 }));
        //    tr.Size({ 50, 50 });
        //    tr.AnchorPoint({ 1, 0 });
        //    tr.RelativeOffsetAdjustment({ 1, 0, 0 });
        //    visuals.InsertAtTop(tr);

        //    auto bl{ m_compositor.CreateSpriteVisual() };
        //    bl.Brush(m_compositor.CreateColorBrush({ 128, 0, 0, 255 }));
        //    bl.Size({ 50, 50 });
        //    bl.AnchorPoint({ 0, 1 });
        //    bl.RelativeOffsetAdjustment({ 0, 1, 0 });
        //    visuals.InsertAtTop(bl);

        //    auto br{ m_compositor.CreateSpriteVisual() };
        //    br.Brush(m_compositor.CreateColorBrush({ 128, 128, 0, 128 }));
        //    br.Size({ 50, 50 });
        //    br.AnchorPoint({ 1, 1 });
        //    br.RelativeOffsetAdjustment({ 1, 1, 0 });
        //    visuals.InsertAtTop(br);

        //    // Try to do some gaussian blur
        //    /*winrt::MGCE::GaussianBlurEffect blurEffect{};
        //    blurEffect.Name(L"Blur");
        //    blurEffect.BlurAmount(5.0f);
        //    blurEffect.BorderMode(winrt::MGCE::EffectBorderMode::Hard);
        //    blurEffect.Optimization(winrt::MGCE::EffectOptimization::Quality);
        //    blurEffect.Source(winrt::WUIC::CompositionEffectSourceParameter{ L"source" });
        //    winrt::WUIC::CompositionEffectFactory blurEffectFactory{
        //        m_compositor.CreateEffectFactory(blurEffect)
        //    };
        //    winrt::WUIC::CompositionEffectBrush blurBrush{
        //        blurEffectFactory.CreateBrush()
        //    };*/

        //    // Or a blending effect
        //    winrt::Windows::UI::Color bgColor{ 255, 128, 128, 128 };
        //    winrt::MGCE::ColorSourceEffect bgColorEffect{};
        //    bgColorEffect.Color(bgColor);

        //    winrt::MGCE::BlendEffect blendEffect{};
        //    blendEffect.Name(L"Blend");
        //    blendEffect.Mode(winrt::MGCE::BlendEffectMode::ColorBurn);
        //    blendEffect.Background(winrt::WUIC::CompositionEffectSourceParameter{ L"source" });
        //    blendEffect.Foreground(bgColorEffect);
        //    winrt::WUIC::CompositionEffectFactory blendEffectFactory{
        //        m_compositor.CreateEffectFactory(blendEffect)
        //    };
        //    winrt::WUIC::CompositionEffectBrush blendBrush{
        //        blendEffectFactory.CreateBrush()
        //    };

        //    auto backdropBrush{ m_compositor.CreateBackdropBrush() };
        //    blendBrush.SetSourceParameter(L"source", backdropBrush);

        //    auto blurVisual{ m_compositor.CreateSpriteVisual() };
        //    blurVisual.Brush(blendBrush);
        //    blurVisual.RelativeSizeAdjustment({ 1, 1 });
        //    blurVisual.AnchorPoint({ 0.5, 0.5 });
        //    blurVisual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });
        //    visuals.InsertAtTop(blurVisual);

        //    // Try to render some text with win2d
        //    winrt::MGCT::CanvasTextFormat textFormat;
        //    textFormat.FontFamily(L"Segoe UI");
        //    textFormat.FontSize(64);
        //    textFormat.FontWeight(winrt::WUIT::FontWeights::Black());
        //    auto textVisual{
        //        std::make_unique<TextVisual>(
        //            m_compositor,
        //            m_canvasDevice,
        //            m_graphicsDevice,
        //            textFormat,
        //            L"Hello World!"
        //        )
        //    };
        //    visuals.InsertAtTop(textVisual->Visual());
        //    m_visuals.push_back(std::move(textVisual));
        //}
    }

    winrt::WUIC::ContainerVisual CompositionHost::CreateRootVisual()
    {
        auto root{ m_compositor.CreateContainerVisual() };
        root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        root.Offset({ 0, 0, 0 });

        // Create background
        // TODO: Allow this to be customized.
        winrt::Windows::UI::Color bgColor{ 255, 128, 128, 128 };
        winrt::MGCE::ColorSourceEffect bgColorEffect{};
        bgColorEffect.Color(bgColor);
        winrt::MGCE::BlendEffect blendEffect{};
        blendEffect.Name(L"Blend");
        blendEffect.Mode(winrt::MGCE::BlendEffectMode::ColorBurn);
        blendEffect.Background(winrt::WUIC::CompositionEffectSourceParameter{ L"source" });
        blendEffect.Foreground(bgColorEffect);
        winrt::WUIC::CompositionEffectFactory blendEffectFactory{
            m_compositor.CreateEffectFactory(blendEffect)
        };
        winrt::WUIC::CompositionEffectBrush blendBrush{
            blendEffectFactory.CreateBrush()
        };
        auto backdropBrush{ m_compositor.CreateBackdropBrush() };
        blendBrush.SetSourceParameter(L"source", backdropBrush);
        auto backgroundVisual{ m_compositor.CreateSpriteVisual() };
        backgroundVisual.Brush(blendBrush);
        backgroundVisual.RelativeSizeAdjustment({ 1, 1 });
        backgroundVisual.AnchorPoint({ 0.5, 0.5 });
        backgroundVisual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });
        root.Children().InsertAtBottom(backgroundVisual);

        return root;
    }

    void CompositionHost::PresentRootVisual(winrt::WUIC::ContainerVisual rootVisual)
    {
        m_target.Root(rootVisual);
    }
#pragma endregion Public
}