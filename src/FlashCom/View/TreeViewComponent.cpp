#include <pch.h>
#include "TreeViewComponent.h"

namespace
{
    constexpr float c_bufferBetweenKeyAndName{ 64 };
    constexpr float c_bufferBetweenNodes{ 16 };

    winrt::MGCT::CanvasTextFormat GenerateKeyTextFormat()
    {
        winrt::MGCT::CanvasTextFormat textFormat;
        textFormat.FontFamily(L"Segoe UI");
        textFormat.FontSize(72);
        textFormat.FontWeight(winrt::WUIT::FontWeights::Black());
        textFormat.HorizontalAlignment(winrt::MGCT::CanvasHorizontalAlignment::Center);
        return textFormat;
    }

    winrt::MGCT::CanvasTextFormat GenerateTitleTextFormat()
    {
        winrt::MGCT::CanvasTextFormat textFormat;
        textFormat.FontFamily(L"Segoe UI");
        textFormat.FontSize(72);
        textFormat.FontWeight(winrt::WUIT::FontWeights::Light());
        textFormat.HorizontalAlignment(winrt::MGCT::CanvasHorizontalAlignment::Left);
        return textFormat;
    }

    winrt::WUIC::SpriteVisual CreateTextVisual(
        FlashCom::View::CompositionManager& compositionManager,
        winrt::MGCT::CanvasTextFormat textFormat, std::string_view content)
    {
        auto compositor{ compositionManager.GetCompositor() };
        auto [drawingBounds, drawingBrush] { compositionManager.CreateTextBrush(
            textFormat, content) };

        // Apply a blend mode
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
    TreeViewComponent::TreeViewComponent(CompositionManager& compositionManager,
        Models::TreeNode const * const nodeModel) : UiComponent(compositionManager),
        m_nodeModel{ nodeModel }, m_nodeKeyTextFormat{ GenerateKeyTextFormat() },
        m_nodeTitleTextFormat{ GenerateTitleTextFormat() }
    {
        GenerateVisual();
    }

    winrt::WUIC::Visual TreeViewComponent::GetVisual() const
    {
        return m_visual;
    }

    void TreeViewComponent::GenerateVisual()
    {
        auto visual{ winrt::make<winrt::WUIC::ContainerVisual>() };

        // Create visuals and calculate sizes
        std::vector<std::pair<winrt::WUIC::Visual, winrt::WUIC::Visual>> nodeVisuals;
        float maxKeyWidth{ 0 };
        float maxNameWidth{ 0 };
        float maxHeight{ 0 };
        for (const auto& childNode : m_nodeModel->GetChildren())
        {
            auto keyTextVisual{ CreateTextVisual(m_compositionManager,
                m_nodeKeyTextFormat, std::format("{}",
                    static_cast<char>(childNode->GetVkCode()))) };
            auto nameTextVisual{ CreateTextVisual(m_compositionManager,
                m_nodeTitleTextFormat, childNode->GetName()) };
            maxKeyWidth = max(keyTextVisual.Size().x, maxKeyWidth);
            maxNameWidth = max(nameTextVisual.Size().x, maxNameWidth);
            maxHeight = max(max(keyTextVisual.Size().y,
                nameTextVisual.Size().y), maxHeight);
            nodeVisuals.emplace_back(keyTextVisual, nameTextVisual);
        }

        // Calculate layout and populate
        float xOffset{ (m_bounds.first / 2.0f) -
            ((maxKeyWidth + maxNameWidth + c_bufferBetweenKeyAndName) / 2.0f) };
        float yOffset{ (m_bounds.second / 2.0f) -
            (((maxHeight * nodeVisuals.size()) +
                (c_bufferBetweenNodes * max((nodeVisuals.size() - 1), 0))) / 2.0f) };
        for (size_t i{ 0 }; i < nodeVisuals.size(); ++i)
        {
            auto& keyVisual{ nodeVisuals.at(i).first };
            auto keyXOffset{ (maxKeyWidth - keyVisual.Size().x) / 2.0f };
            auto& textVisual{ nodeVisuals.at(i).second };
            float nodeYOffset{ yOffset + (i * c_bufferBetweenNodes) + (i * maxHeight) };
            keyVisual.Offset({ (xOffset + keyXOffset), nodeYOffset, 0 });
            textVisual.Offset({ (xOffset + maxKeyWidth + c_bufferBetweenKeyAndName),
                nodeYOffset, 0 });
            visual.Children().InsertAtTop(keyVisual);
            visual.Children().InsertAtTop(textVisual);
        }

        m_visual = visual;
    }
}