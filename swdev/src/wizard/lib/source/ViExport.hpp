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

#ifndef VIEXPORT_HPP
#define VIEXPORT_HPP

#if defined(_WIN32)

#define VI_DEPRECATED __declspec(deprecated)
#define VI_DLL_EXPORT __declspec(dllexport)
#define VI_DLL_IMPORT __declspec(dllimport)

// Prevent compiler warnings about dll-interface with templates.
#pragma warning(disable : 4251)
#ifdef wizard_core_EXPORTS
#define VI_BUILD_DLL
#endif
#if !defined(VI_BUILD_DLL) && defined(SWDEV_ALL_USE_DLL)
#define VI_USE_DLL
#endif

#if defined(VI_BUILD_DLL) // Creating DLL (i.e. exporting)
#define VI_EXPORT __declspec(dllexport)
#elif defined(VI_USE_DLL) // Using DLL (i.e. importing).
#define VI_EXPORT __declspec(dllimport)
#else
#define VI_EXPORT // Compiling statically.
#endif
#else
#define VI_EXPORT
#define VI_DEPRECATED __attribute__((deprecated))
#define VI_DLL_EXPORT
#define VI_DLL_IMPORT
#endif

#endif // ViExport_hpp
