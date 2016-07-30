#pragma once

#define ELPP_UNICODE
#define ELPP_NO_DEFAULT_LOG_FILE

#include "easylogging++.h"

template <class Config_>
class EasyLoggingT
{
private:
    el::Configurations  m_logConfig;

public:
    typedef Config_ Config;

    EasyLoggingT();

private:
    static const char* getIp();
};


template <class T>
EasyLoggingT<T>::EasyLoggingT() {
    /// @todo doug: initialize logger here
    el::Helpers::installCustomFormatSpecifier(el::CustomFormatSpecifier("%ip_addr", getIp));
    m_logConfig.setGlobally(el::ConfigurationType::Format,
                            "%datetime %level %user %host %ip_addr %fbase:%line %func %msg");

    /// @todo doug: Create an installation directory and write the log file to,
    // say %CONFIG_DIR%\log\win32Service.log (or some file in that directory).
    m_logConfig.setGlobally(el::ConfigurationType::Filename,
                            "win32Service.log");    /// @note: Services will write files to C:\Windows\system32.

    // Log to file only.
    m_logConfig.setGlobally(el::ConfigurationType::ToFile, "true");
    m_logConfig.setGlobally(el::ConfigurationType::ToStandardOutput, "false");

    el::Loggers::addFlag(el::LoggingFlag::ImmediateFlush);

    // Set the default logger to use our configuration
    el::Loggers::reconfigureLogger("default", m_logConfig);
    LOG(INFO) << L"Log using the default file";
}