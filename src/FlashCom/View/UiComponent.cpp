#include <pch.h>
#include "UiComponent.h"

namespace FlashCom::View
{
    UiComponent::UiComponent(CompositionManager& compositionManager) :
        m_compositionManager{ compositionManager }
    { }
}