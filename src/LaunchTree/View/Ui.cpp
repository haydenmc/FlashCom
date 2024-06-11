#include <pch.h>
#include "Ui.h"

namespace LaunchTree::View
{
    Ui::Ui(CompositionHost* const compositionHost,
        Models::DataModel const * const dataModel) :
        m_dataModel{ dataModel },
        m_compositionHost{ compositionHost }
    { }

    void Ui::CreateView()
    {

    }
}