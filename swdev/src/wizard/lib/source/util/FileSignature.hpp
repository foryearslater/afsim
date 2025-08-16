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
#ifndef FILESIGNATURE_HPP
#define FILESIGNATURE_HPP
#include <cstdlib>
#include <ctime>

#include "ViExport.hpp"
class UtPath;

namespace wizard
{
class VI_EXPORT FileSignature
{
public:
   FileSignature();
   enum CompareResult
   {
      cNO_CHANGE      = 0,
      cTIME_CHANGE    = 1,
      cFILE_NOT_FOUND = 8
      // cDATA_CHANGE = 3
   };

   static FileSignature GetSignature(UtPath& aPath);
   int                  Update(const UtPath& aPath);
   size_t               mPreviousFileSizeStat;
   time_t               mPreviousFileDateStat;
   // std::string mHash;
};
} // namespace wizard
#endif
