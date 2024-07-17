#pragma once
#include "CompositionHost.h"
#include <Models/DataModel.h>

namespace LaunchTree::View
{
    class Ui
    {
    public:
        Ui(HostWindow& hostWindow,
            Models::DataModel const * const dataModel);
        void Show();
        void Hide();
        void Update();

    private:
        HostWindow& m_hostWindow;
        Models::DataModel const * const m_dataModel;
        CompositionHost m_compositionHost;
        std::pair<uint32_t, uint32_t> m_uiBounds;
    };
}