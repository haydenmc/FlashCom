#pragma once

// Windows API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// C++/WinRT
#include <Unknwn.h>
#include <winrt/base.h>

// Spdlog
#ifdef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO // Compile out debug/trace logs in non-debug builds
#endif
#include <spdlog/spdlog.h>
