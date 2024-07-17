#pragma once

// Windows API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// C++/WinRT
#include <Unknwn.h>
#include <winrt/base.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Effects.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Composition.h>
#include <winrt/Windows.Graphics.Effects.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.UI.Text.h>

// JSON
#include <json.hpp>

// Namespace aliases
namespace winrt
{
    namespace MGC = Microsoft::Graphics::Canvas;
    namespace MGCE = Microsoft::Graphics::Canvas::Effects;
    namespace MGCT = Microsoft::Graphics::Canvas::Text;
    namespace MGCUC = Microsoft::Graphics::Canvas::UI::Composition;
    namespace WS = Windows::System;
    namespace WUIC = Windows::UI::Composition;
    namespace WUICD = Windows::UI::Composition::Desktop;
    namespace WUIT = Windows::UI::Text;
}