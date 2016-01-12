/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#pragma once

#include "win32_appT.h"
#include "win32_serviceT.h"
#include "win32_serviceCtrlT.h"


struct ServiceConfig
{
    typedef ServiceConfig               Config;
    typedef AppT<Config>                Service;
    typedef Win32ServiceT<Service>      Win32Service;
    typedef Win32ServiceCtrlT<Service>  Win32ServiceCtrl;
};

typedef ServiceConfig::Service            Service;
typedef ServiceConfig::Win32Service       Win32Service;
typedef ServiceConfig::Win32ServiceCtrl   Win32ServiceCtrl;
