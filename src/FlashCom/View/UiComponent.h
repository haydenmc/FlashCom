#pragma once
#include "CompositionManager.h"
#include <Models/DataModel.h>
#include "Ui.h"

namespace FlashCom::View
{
    struct UiComponent
    {
        UiComponent(CompositionManager& compositionManager);
        virtual winrt::WUIC::Visual GetVisual() const = 0;

    protected:
        CompositionManager& m_compositionManager;
    };
}