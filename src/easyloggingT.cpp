#include "win32_serviceConfig.h"

INITIALIZE_EASYLOGGINGPP;

template <>
const char* EasyLoggingT<ServiceConfig>::getIp() {
    /// @todo doug: return a real IP address
    static const char* result = "127.0.0.1";

    return result;
}
