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

//
// This file contains code that shows a console on windows.  We compile as a 'windows'
// application instead of 'console', so we must explicitly show the console if it is desired.
//
// If the user closes the console, it will terminate the application.  To avoid this problem,
// we usually run without a console in release mode.
//


#include <fstream>
#include <iostream>
#include <stdio.h>

#include <windows.h>

#include <fcntl.h>
#include <io.h>

// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 5000;
void              RedirectIOToConsole()
{
   // int hConHandle;
   // long lStdHandle;
   // CONSOLE_SCREEN_BUFFER_INFO coninfo;
   // FILE *fp;

   //// allocate a console for this app
   // AllocConsole();

   //// set the screen buffer to be big enough to let us scroll text
   // GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
   // coninfo.dwSize.Y = MAX_CONSOLE_LINES;
   // SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

   //// redirect unbuffered STDOUT to the console
   // lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
   // hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   // fp = _fdopen( hConHandle, "w" );
   //*stdout = *fp;
   // setvbuf( stdout, NULL, _IONBF, 0 );

   //// redirect unbuffered STDIN to the console
   // lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
   // hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   // fp = _fdopen( hConHandle, "r" );
   //*stdin = *fp;
   // setvbuf( stdin, NULL, _IONBF, 0 );

   //// redirect unbuffered STDERR to the console
   // lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
   // hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   // fp = _fdopen( hConHandle, "w" );
   //*stderr = *fp;
   // setvbuf( stderr, NULL, _IONBF, 0 );

   //// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
   //// point to console as well
   // ios::sync_with_stdio();

   bool has_console = ::AttachConsole(ATTACH_PARENT_PROCESS) == TRUE;
   if (!has_console)
   {
      // We weren't launched from a console, so make one.
      has_console = AllocConsole() == TRUE;
   }
   if (!has_console)
   {
      return;
   }

   for (auto& file : {stdout, stderr})
   {
      if (freopen("CONOUT$", "w", file))
      {
         setvbuf(file, nullptr, _IONBF, 0);
      }
   }
}

#else
void RedirectIOToConsole()
{
   // nothing
}
#endif
