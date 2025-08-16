// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "DebugPlatform.hpp"

// All of the platform-specific, non-Qt code is shunted off to this file to
// mitigate how much this platform-specific code influences the rest of the
// code base (i.e., no windows.h include anywhere other than here).

//#ifndef Q_OS_WIN
//#error PlatformDebug currently only is implemented for Windows.  Support for this platform needs to be added.
//#endif

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>


namespace PatternVisualizer
{
namespace Debug
{

void DebuggerBreak()
{
   //         __debugbreak();
}

void FallbackShowMessage(QTextDocument* aErrorMessage, const QString& aApplicationTitle, bool aIsFatal)
{
   QString errorMessagePlainText = aErrorMessage->toPlainText();

   //         MessageBoxA(nullptr, errorMessagePlainText.toUtf8(), applicationTitle.toUtf8(), MB_ICONEXCLAMATION | MB_OK);

   if (aIsFatal)
   {
      exit(EXIT_FAILURE);
   }
}
} // namespace Debug
} // namespace PatternVisualizer
