/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#pragma once
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif


// This is meant to be a platform-independent way to represent a file handle. Think about it.
typedef struct PlatformHandle
{
    void *h;
} PlatformHandle;


u16 fletcher16Optimized(u08 const *data, memory_index bytes);


#ifdef __cplusplus
}
#endif
