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

#ifndef WEAPONTOOLSEXPORT_HPP
#define WEAPONTOOLSEXPORT_HPP

#if defined(_WIN32)

#pragma warning(disable : 4251)
#pragma warning(disable : 4786)

#if defined(WEAPON_TOOLS_BUILD_PLUGIN) // Creating DLL (i.e. exporting)
#define WT_EXPORT __declspec(dllexport)
#else
#define WT_EXPORT // Compiling statically.
#endif
#else
#define WT_EXPORT
#endif

#endif
