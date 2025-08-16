// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "UtException.hpp"
#include "ViExport.hpp"
#include "wizard_version_defines.hpp"

// this code is provided by Nvidia.  It forces the use of a laptop's GPU when
// there is also an integrated CPU/GPU present.
#ifdef _WIN32
#include <windows.h>
extern "C"
{
   _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

extern int VI_EXPORT wizard_main(int argc, char* argv[], int, int, int, const char*, const char*, const char*, const char*);
extern "C" int       main(int argc, char* argv[])
{
   ut::SetupApplicationLog("Wizard", WIZARD_VERSION, "wizard-exception.log");
   return wizard_main(argc,
                      argv,
                      WIZARD_VERSION_MAJOR,
                      WIZARD_VERSION_MINOR,
                      WIZARD_VERSION_PATCH,
                      WIZARD_COMPANYNAME_STR,
                      WIZARD_COMPANYDOMAIN_STR,
                      WIZARD_PRODUCTNAME_STR,
                      WIZARD_PRODUCTVERSION_STR);
}
