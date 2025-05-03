#pragma once
#include "UiComponent.h"

#include <Models/TreeNode.h>

namespace FlashCom::View
{
    struct TreeViewComponent : public UiComponent
    {
        TreeViewComponent(CompositionManager& compositionManager,
            Models::TreeNode const * const nodeModel);
        winrt::WUIC::Visual GetVisual() const override;

    private:
        Models::TreeNode const * const m_nodeModel;
        winrt::WUIC::Visual m_visual{ nullptr };
        winrt::MGCT::CanvasTextFormat m_nodeKeyTextFormat;
        winrt::MGCT::CanvasTextFormat m_nodeTitleTextFormat;

        void GenerateVisual();
    };
}