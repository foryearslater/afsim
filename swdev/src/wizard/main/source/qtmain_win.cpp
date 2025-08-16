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

#ifdef _WIN32
/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Windows main function of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <shlobj.h>

#include "qbytearray.h"
#include "qstring.h"
#include "qt_windows.h"
#include "qvector.h"

/*
This file contains the code in the qtmain library for Windows.
qtmain contains the Windows startup code and is required for
linking to the Qt DLL.

When a Windows application starts, the WinMain function is
invoked.
*/

QT_USE_NAMESPACE


#if defined(QT_NEEDS_QMAIN)
int qMain(int, char**);
#define main qMain
#else
extern "C" int main(int, char**);
#endif

/*
WinMain() - Initializes Windows and calls user's startup function main().
NOTE: WinMain() won't be called if the application was linked as a "console"
application.
*/

// Convert a wchar_t to char string, equivalent to QString::toLocal8Bit()
// when passed CP_ACP.
static inline char* wideToMulti(int codePage, const wchar_t* aw)
{
   const int required = WideCharToMultiByte(codePage, 0, aw, -1, NULL, 0, NULL, NULL);
   char*     result   = new char[required];
   WideCharToMultiByte(codePage, 0, aw, -1, result, required, NULL, NULL);
   return result;
}

extern "C" int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /* cmdShow */)
{
   int       argc;
   wchar_t** argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
   if (!argvW)
      return -1;
   char** argv = new char*[argc + 1];
   for (int i = 0; i < argc; ++i)
      argv[i] = wideToMulti(CP_ACP, argvW[i]);
   argv[argc] = Q_NULLPTR;
   LocalFree(argvW);
   const int exitCode = main(argc, argv);
   for (int i = 0; i < argc && argv[i]; ++i)
      delete[] argv[i];
   delete[] argv;
   return exitCode;
}
#endif
