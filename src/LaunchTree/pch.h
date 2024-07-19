#pragma once

// Windows API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

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

// External libraries
#pragma warning(push, 0)
#include <json.hpp>
#define UTF_CPP_CPLUSPLUS _MSVC_LANG // Fix for UTF8 C++ std version
                                     // https://github.com/nemtrif/utfcpp/issues/85
#include <utf8.h>
#pragma warning(pop)

// Resources
#include "resource.h"

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