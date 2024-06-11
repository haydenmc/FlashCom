#pragma once
#include "CompositionHost.h"
#include <Models/DataModel.h>

namespace LaunchTree::View
{
    class Ui
    {
    public:
        Ui(CompositionHost* const compositionHost,
            Models::DataModel const * const dataModel);

        void CreateView();

    private:
        Models::DataModel const * const m_dataModel;
        CompositionHost* const m_compositionHost;
    };
}