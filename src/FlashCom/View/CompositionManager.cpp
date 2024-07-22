#include <pch.h>
#include "CompositionManager.h"
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
        HWND window, winrt::WUIC::Compositor compositor)
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
    CompositionManager::CompositionManager(HostWindow const& hostWindow) :
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

    winrt::Windows::UI::Composition::Compositor CompositionManager::GetCompositor()
    {
        return m_compositor;
    }

    void CompositionManager::PresentRootVisual(winrt::WUIC::ContainerVisual rootVisual)
    {
        m_target.Root(rootVisual);
    }

    BrushWithBounds CompositionManager::CreateTextBrush(
        winrt::MGCT::CanvasTextFormat textFormat, std::string_view content)
    {
        // Compute bounds of text
        winrt::MGCT::CanvasTextLayout textLayout{
            m_canvasDevice,
            winrt::to_hstring(content),
            textFormat,
            0,
            0
        };
        textLayout.WordWrapping(winrt::MGCT::CanvasWordWrapping::NoWrap);
        auto textBounds{ textLayout.LayoutBounds() };

        // Draw text
        auto drawingSurface{
            m_graphicsDevice.CreateDrawingSurface(
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
        return {
            .Bounds = drawingSurface.Size(),
            .Brush = m_compositor.CreateSurfaceBrush(drawingSurface)
        };
    }
#pragma endregion Public
}