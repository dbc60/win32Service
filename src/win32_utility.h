#if !defined(WIN32_UTILITY_H)
/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#if !defined(PLATFORM_H)
#include "platform.h"
#endif
#include <string>

std::wstring Win32ErrorToWString(u32 errorCode);


#define WIN32_UTILITY_H
#endif
