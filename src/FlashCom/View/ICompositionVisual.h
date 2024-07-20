#pragma once
#include <winrt/Windows.UI.Composition.h>

namespace FlashCom::View
{
    class ICompositionVisual
    {
    public:
        virtual ~ICompositionVisual() { };
        virtual winrt::Windows::UI::Composition::Visual Visual() = 0;
    };
}
