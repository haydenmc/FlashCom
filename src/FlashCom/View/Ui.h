#pragma once
#include "CompositionManager.h"
#include <Models/DataModel.h>

namespace FlashCom::View
{
    enum class UpdateReasonKind
    {
        Reloading,
        Showing,
        Hiding,
        Navigating,
    };

    class Ui
    {
    public:
        Ui(HostWindow& hostWindow,
            Models::DataModel const * const dataModel);
        void Show();
        void Hide();
        void Update(UpdateReasonKind reason);

    private:
        HostWindow& m_hostWindow;
        Models::DataModel const * const m_dataModel;
        CompositionManager m_compositionManager;
        std::pair<uint32_t, uint32_t> m_uiBounds;
        winrt::Windows::UI::Composition::ContainerVisual m_backgroundVisual{ nullptr };
        winrt::Windows::UI::Composition::ContainerVisual m_contentsVisual{ nullptr };
        winrt::Windows::UI::Composition::ContainerVisual m_rootVisual{ nullptr };
        // Animations
        winrt::Windows::UI::Composition::ScalarKeyFrameAnimation
            m_backgroundFadeInAnimation{ nullptr };
        winrt::Windows::UI::Composition::Vector3KeyFrameAnimation
            m_contentScaleInAnimation{ nullptr };
    };
}