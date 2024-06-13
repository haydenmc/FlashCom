#include <pch.h>
#include "Ui.h"

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
        m_hostWindow.ShowAndForceToForeground();
    }

    void Ui::Hide()
    {
        m_hostWindow.Hide();
    }

    void Ui::Update()
    {
        auto rootVisual{ m_compositionHost.CreateRootVisual() };
        m_compositionHost.PresentRootVisual(rootVisual);
    }
#pragma endregion Public
}