#include <pch.h>
#include "Logging.h"

void InitializeLogging(std::string_view logFilePath)
{
    static const bool s_loggingInitialized{ [&logFilePath]() {
        std::vector<spdlog::sink_ptr> sinks;
    #ifdef _DEBUG
        auto msvcSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() };
        sinks.push_back(msvcSink);
    #endif
        auto fileSink{ std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string{ logFilePath }) };
        sinks.push_back(fileSink);

        auto logger{ std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end()) };
#ifdef _DEBUG
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);
#else
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
#endif
        spdlog::set_default_logger(logger);
        SPDLOG_INFO("Logging initialized.");
        return true;
    }() };
}