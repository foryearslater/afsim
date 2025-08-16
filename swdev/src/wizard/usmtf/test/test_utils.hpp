// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#ifndef TESTUTILS
#define TESTUTILS
#include <fstream>


// Simple Struct to organize useful result info
struct FileCompareResult
{
   FileCompareResult(bool aWasIdentical = false, char aExpected = ' ', char aActual = ' ', std::streampos aPos = -1)
      : expected{aExpected}
      , actual{aActual}
      , position{aPos}
      , wasIdentical{aWasIdentical}
   {
   }
   char           expected;
   char           actual;
   std::streampos position;
   bool           wasIdentical;
};

// This is for byte by byte comparison of files for regression testing accuracy
// Bare bones error reporting on this. just char and position, no context
FileCompareResult AreFilesIdentical(std::ifstream& f1, std::ifstream& f2);

#endif
