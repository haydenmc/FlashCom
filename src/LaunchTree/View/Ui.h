#pragma once
#include "CompositionHost.h"

namespace LaunchTree::View
{
    class Ui
    {
    public:
        Ui(CompositionHost* compositionHost);

    private:
        CompositionHost* const m_compositionHost;
    };
}