#include <pch.h>
#include "Ui.h"

namespace
{
    constexpr float c_bufferBetweenKeyAndName{ 64 };
    constexpr float c_bufferBetweenNodes{ 16 };
    constexpr std::chrono::milliseconds c_backgroundFadeInAnimationTime{ 250 };
    constexpr std::chrono::milliseconds c_contentScaleInAnimationTime{ 500 };
    
    winrt::WUIC::ScalarKeyFrameAnimation CreateBackgroundFadeInAnimation(
        winrt::WUIC::Compositor compositor)
    {
        auto animation{ compositor.CreateScalarKeyFrameAnimation() };
        auto easingFunction{
            winrt::WUIC::CompositionEasingFunction::CreateCircleEasingFunction(
                compositor, winrt::WUIC::CompositionEasingFunctionMode::Out) };
        animation.InsertKeyFrame(0.0f, 0.0f, easingFunction);
        animation.InsertKeyFrame(1.0f, 1.0f, easingFunction);
        animation.Duration(c_backgroundFadeInAnimationTime);
        return animation;
    }

    winrt::WUIC::Vector3KeyFrameAnimation CreateContentScaleInAnimation(
        winrt::WUIC::Compositor compositor)
    {
        auto animation{ compositor.CreateVector3KeyFrameAnimation() };
        auto easingFunction{
            winrt::WUIC::CompositionEasingFunction::CreatePowerEasingFunction(
                compositor, winrt::WUIC::CompositionEasingFunctionMode::Out, 5) };
        animation.InsertKeyFrame(0.0f, { 0.8f, 0.8f, 0.8f }, easingFunction);
        animation.InsertKeyFrame(1.0f, { 1.0f, 1.0f, 1.0f }, easingFunction);
        animation.Duration(c_contentScaleInAnimationTime);
        return animation;
    }

    winrt::WUIC::ContainerVisual CreateBackgroundVisual(winrt::WUIC::Compositor compositor)
    {
        // Create background
        // TODO: Allow this to be customized.
        winrt::Windows::UI::Color bgColor{ 192, 128, 128, 128 };
        winrt::MGCE::ColorSourceEffect bgColorEffect{};
        bgColorEffect.Color(bgColor);
        winrt::MGCE::BlendEffect blendEffect{};
        blendEffect.Name(L"Blend");
        blendEffect.Mode(winrt::MGCE::BlendEffectMode::LinearBurn);
        blendEffect.Background(winrt::WUIC::CompositionEffectSourceParameter{ L"source" });
        blendEffect.Foreground(bgColorEffect);
        winrt::WUIC::CompositionEffectFactory blendEffectFactory{
            compositor.CreateEffectFactory(blendEffect)
        };
        winrt::WUIC::CompositionEffectBrush blendBrush{
            blendEffectFactory.CreateBrush()
        };
        auto backdropBrush{ compositor.CreateBackdropBrush() };
        blendBrush.SetSourceParameter(L"source", backdropBrush);
        auto backgroundVisual = compositor.CreateSpriteVisual();
        backgroundVisual.Brush(blendBrush);
        backgroundVisual.RelativeSizeAdjustment({ 1, 1 });
        backgroundVisual.AnchorPoint({ 0.5, 0.5 });
        backgroundVisual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });

        return backgroundVisual;
    }

    winrt::WUIC::ContainerVisual CreateContentsVisual(winrt::WUIC::Compositor compositor)
    {
        auto visual{ compositor.CreateContainerVisual() };
        visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
        visual.AnchorPoint({ 0.5f, 0.5f });
        visual.RelativeOffsetAdjustment({ 0.5, 0.5, 0 });
        return visual;
    }

    winrt::WUIC::ContainerVisual CreateRootVisual(winrt::WUIC::Compositor compositor,
        winrt::WUIC::ContainerVisual backgroundVisual, winrt::WUIC::ContainerVisual contentsVisual)
    {
        auto root{ compositor.CreateContainerVisual() };
        root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        root.Offset({ 0, 0, 0 });
        root.Children().InsertAtBottom(backgroundVisual);
        root.Children().InsertAtTop(contentsVisual);
        return root;
    }

    winrt::WUIC::ContainerVisual CreateClockContainer(winrt::WUIC::Compositor compositor)
    {
        auto clockContainer{ compositor.CreateContainerVisual() };
        clockContainer.RelativeSizeAdjustment({ 1.0f, 1.0f });
        clockContainer.Offset({ 0, 0, 0 });
        return clockContainer;
    }

    winrt::WUIC::SpriteVisual CreateTextVisual(
        FlashCom::View::CompositionManager& compositionManager,
        winrt::MGCT::CanvasTextFormat textFormat, std::string_view content)
    {
        auto compositor{ compositionManager.GetCompositor() };
        auto [drawingBounds, drawingBrush] { compositionManager.CreateTextBrush(
            textFormat, content) };

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
        spriteVisual.Size(drawingBounds);

        return spriteVisual;
    }
}

namespace FlashCom::View
{
#pragma region Public
    Ui::Ui(HostWindow& hostWindow,
        Models::DataModel const* const dataModel) :
        m_hostWindow{ hostWindow },
        m_dataModel{ dataModel },
        m_compositionManager{ hostWindow },
        m_backgroundVisual{ CreateBackgroundVisual(m_compositionManager.GetCompositor()) },
        m_contentsVisual{ CreateContentsVisual(m_compositionManager.GetCompositor()) },
        m_rootVisual{ CreateRootVisual(m_compositionManager.GetCompositor(), m_backgroundVisual,
            m_contentsVisual) },
        m_clockVisual{ CreateClockContainer(m_compositionManager.GetCompositor()) },
        m_backgroundFadeInAnimation{
            CreateBackgroundFadeInAnimation(m_compositionManager.GetCompositor()) },
        m_contentScaleInAnimation{ CreateContentScaleInAnimation(
            m_compositionManager.GetCompositor()) }
    {
        m_compositionManager.PresentRootVisual(m_rootVisual);
    }

    void Ui::Show()
    {
        auto showWindowTicket{ m_hostWindow.PrepareToShow() };
        m_uiBounds = showWindowTicket.WindowSize;
        Update(UpdateReasonKind::Showing);
        m_compositionManager.GetCompositor().RequestCommitAsync().Completed(
            [showWindowTicket](winrt::WF::IAsyncAction, winrt::WF::AsyncStatus)
            {
                showWindowTicket.ShowWindow();
            });
    }

    void Ui::Hide()
    {
        m_hostWindow.Hide();
    }

    void Ui::Update(UpdateReasonKind reason)
    {
        // Right now, we do a full re-population of visuals on every update.
        // In the future maybe we can diff and such.
        m_contentsVisual.Children().RemoveAll();

        // Create text visuals for each child node
        winrt::MGCT::CanvasTextFormat nameTextFormat;
        nameTextFormat.FontFamily(L"Segoe UI");
        nameTextFormat.FontSize(72);
        nameTextFormat.FontWeight(winrt::WUIT::FontWeights::SemiLight());
        winrt::MGCT::CanvasTextFormat keyTextFormat;
        keyTextFormat.FontFamily(L"Segoe UI");
        keyTextFormat.FontSize(72);
        keyTextFormat.FontWeight(winrt::WUIT::FontWeights::Black());
        keyTextFormat.HorizontalAlignment(winrt::MGCT::CanvasHorizontalAlignment::Left);
        std::vector<std::pair<winrt::WUIC::Visual, winrt::WUIC::Visual>> nodeVisuals;
        float maxKeyWidth{ 0 };
        float maxNameWidth{ 0 };
        float maxHeight{ 0 };
        for (const auto& childNode : m_dataModel->CurrentNode->GetChildren())
        {
            auto keyTextVisual{ CreateTextVisual(m_compositionManager,
                keyTextFormat, std::format("{}", static_cast<char>(childNode->GetVkCode())))};
            auto nameTextVisual{ CreateTextVisual(m_compositionManager,
                nameTextFormat, childNode->GetName()) };
            maxKeyWidth = max(keyTextVisual.Size().x, maxKeyWidth);
            maxNameWidth = max(nameTextVisual.Size().x, maxNameWidth);
            maxHeight = max(max(keyTextVisual.Size().y,
                nameTextVisual.Size().y), maxHeight);
            nodeVisuals.emplace_back(keyTextVisual, nameTextVisual);
        }

        // Layout
        float xOffset{ (m_uiBounds.first / 2.0f) -
            ( (maxKeyWidth + maxNameWidth + c_bufferBetweenKeyAndName) / 2.0f ) };
        float yOffset{ (m_uiBounds.second / 2.0f) -
            ( ( (maxHeight * nodeVisuals.size()) +
                (c_bufferBetweenNodes * max((nodeVisuals.size() - 1), 0) ) ) / 2.0f ) };
        for (size_t i{ 0 }; i < nodeVisuals.size(); ++i)
        {
            auto& keyVisual{ nodeVisuals.at(i).first };
            auto keyXOffset{ (maxKeyWidth - keyVisual.Size().x) / 2.0f };
            auto& textVisual{ nodeVisuals.at(i).second };
            float nodeYOffset{ yOffset + (i * c_bufferBetweenNodes) + (i * maxHeight) };
            keyVisual.Offset({ (xOffset + keyXOffset), nodeYOffset, 0 });
            textVisual.Offset({ (xOffset + maxKeyWidth + c_bufferBetweenKeyAndName),
                nodeYOffset, 0 });
            m_contentsVisual.Children().InsertAtTop(keyVisual);
            m_contentsVisual.Children().InsertAtTop(textVisual);
        }

        // Clock visual
        UpdateClockVisual();
        m_contentsVisual.Children().InsertAtTop(m_clockVisual);

        // If we're showing, prepare to animate in
        if (reason == UpdateReasonKind::Showing)
        {
            m_backgroundVisual.StartAnimation(L"opacity", m_backgroundFadeInAnimation);
            m_contentsVisual.StartAnimation(L"opacity", m_backgroundFadeInAnimation);
            m_contentsVisual.StartAnimation(L"scale", m_contentScaleInAnimation);
        }
    }

    void Ui::UpdateClockVisual()
    {
        static auto const locale{ std::locale("") };
        auto const time = std::chrono::current_zone()
            ->to_local(std::chrono::system_clock::now());

        std::string timeText;
        if (m_dataModel->UseTwentyFourHourClock)
        {
            timeText = std::format(locale, "{0:%H}:{0:%M}", time);
        }
        else
        {
            timeText = std::format(locale, "{0:%I}:{0:%M}", time);
        }

        winrt::MGCT::CanvasTextFormat timeTextFormat;
        timeTextFormat.FontFamily(L"Segoe UI");
        timeTextFormat.FontSize(128);
        timeTextFormat.FontWeight(winrt::WUIT::FontWeights::Bold());
        timeTextFormat.HorizontalAlignment(winrt::MGCT::CanvasHorizontalAlignment::Left);
        auto timeVisual{ CreateTextVisual(m_compositionManager, timeTextFormat, timeText) };
        timeVisual.AnchorPoint({0.0f, 1.0f}); 
        timeVisual.RelativeOffsetAdjustment({0.0f, 1.0f, 0.0f}); 

        auto dateText{ std::format(locale, "{:%x}", time) };
        winrt::MGCT::CanvasTextFormat dateTextFormat;
        dateTextFormat.FontFamily(L"Segoe UI");
        dateTextFormat.FontSize(32);
        dateTextFormat.FontWeight(winrt::WUIT::FontWeights::Light());
        dateTextFormat.HorizontalAlignment(winrt::MGCT::CanvasHorizontalAlignment::Left);
        auto dateVisual{ CreateTextVisual(m_compositionManager, dateTextFormat, dateText) };
        dateVisual.AnchorPoint({ 0.0f, 1.0f });
        dateVisual.RelativeOffsetAdjustment({ 0.0f, 1.0f, 0.0f });
        // HACK: Until we get text layouts fixed, bump the y position up a
        // little bit to line it up with the time
        dateVisual.Offset({ (timeVisual.Size().x + 4.0f), -24.0f, 0.0f });
        
        m_clockVisual.Children().RemoveAll();
        m_clockVisual.Children().InsertAtTop(timeVisual);
        m_clockVisual.Children().InsertAtTop(dateVisual);

    }
#pragma endregion Public
}