#include <pch.h>
#include "TextVisual.h"
#include "Ui.h"

namespace
{
    constexpr float c_bufferBetweenKeyAndName{ 32 };
    constexpr float c_bufferBetweenNodes{ 32 };
}

namespace LaunchTree::View
{
#pragma region Public
    Ui::Ui(HostWindow& hostWindow,
        Models::DataModel const * const dataModel) :
        m_hostWindow{ hostWindow },
        m_dataModel{ dataModel },
        m_compositionHost{ hostWindow }
    { }

    void Ui::Show()
    {
        auto showWindowTicket{ m_hostWindow.PrepareToShow() };
        m_uiBounds = showWindowTicket.WindowSize;
        Update();
        showWindowTicket.ShowWindow();
    }

    void Ui::Hide()
    {
        m_hostWindow.Hide();
    }

    void Ui::Update()
    {
        auto rootVisual{ m_compositionHost.CreateRootVisual() };
        // Create text visuals for each child node
        winrt::MGCT::CanvasTextFormat nameTextFormat;
        nameTextFormat.FontFamily(L"Segoe UI");
        nameTextFormat.FontSize(64);
        nameTextFormat.FontWeight(winrt::WUIT::FontWeights::Normal());
        winrt::MGCT::CanvasTextFormat keyTextFormat;
        keyTextFormat.FontFamily(L"Segoe UI");
        keyTextFormat.FontSize(64);
        keyTextFormat.FontWeight(winrt::WUIT::FontWeights::Black());
        std::vector<std::pair<winrt::WUIC::Visual, winrt::WUIC::Visual>> nodeVisuals;
        float maxKeyWidth{ 0 };
        float maxNameWidth{ 0 };
        float maxHeight{ 0 };
        for (const auto& childNode : m_dataModel->CurrentNode->GetChildren())
        {
            auto keyTextVisual{ m_compositionHost.CreateVisual<TextVisual>(
                keyTextFormat, std::format(L"{}", static_cast<char>(childNode->GetVkCode())))};
            auto nameTextVisual{ m_compositionHost.CreateVisual<TextVisual>(
                nameTextFormat, childNode->GetName()) };
            maxKeyWidth = max(keyTextVisual->Visual().Size().x, maxKeyWidth);
            maxNameWidth = max(nameTextVisual->Visual().Size().x, maxNameWidth);
            maxHeight = max(max(keyTextVisual->Visual().Size().y,
                nameTextVisual->Visual().Size().y), maxHeight);
            nodeVisuals.emplace_back(keyTextVisual->Visual(), nameTextVisual->Visual());
        }

        // Layout
        float xOffset{ (m_uiBounds.first / 2.0f) -
            ( (maxKeyWidth + maxNameWidth + c_bufferBetweenKeyAndName) / 2.0f ) };
        float yOffset{ (m_uiBounds.second / 2.0f) -
            ( ( (maxHeight * nodeVisuals.size()) +
                (c_bufferBetweenNodes * min((nodeVisuals.size() - 1), 0) ) ) / 2.0f ) };
        for (size_t i{ 0 }; i < nodeVisuals.size(); ++i)
        {
            auto& keyVisual{ nodeVisuals.at(i).first };
            auto& textVisual{ nodeVisuals.at(i).second };
            float nodeYOffset{ yOffset + (i * c_bufferBetweenNodes) + (i * maxHeight) };
            keyVisual.Offset({ xOffset, nodeYOffset, 0 });
            textVisual.Offset({ (xOffset + maxKeyWidth + c_bufferBetweenKeyAndName),
                nodeYOffset, 0 });
            rootVisual.Children().InsertAtTop(keyVisual);
            rootVisual.Children().InsertAtTop(textVisual);
        }

        m_compositionHost.PresentRootVisual(rootVisual);
    }
#pragma endregion Public
}