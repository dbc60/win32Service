#pragma once

#include "easylogging++.h"

template <class Config_>
class EasyLoggingT
{
public:
    typedef Config_ Config;

    EasyLoggingT();
};


template <class T>
EasyLoggingT<T>::EasyLoggingT() {
    /// @todo doug: initialize logger here
}