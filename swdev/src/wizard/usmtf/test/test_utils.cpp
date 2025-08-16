// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "test_utils.hpp"

FileCompareResult AreFilesIdentical(std::ifstream& f1, std::ifstream& f2)
{
   if (!f1 || !f2)
   {
      return {false};
   }

   while (!f1.eof() || !f2.eof())
   {
      char c1 = f1.get();
      char c2 = f2.get();
      if (c1 != c2)
      {
         return {false, c1, c2, f1.tellg()};
      }
   }
   return {true};
}
