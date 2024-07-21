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
    { }

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
        blendEffect.Mode(winrt::MGCE::BlendEffectMode::LinearBurn);
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