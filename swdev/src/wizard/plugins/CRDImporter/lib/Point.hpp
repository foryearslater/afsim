// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef POINT_HPP
#define POINT_HPP

#include <string>

#include "ElementBase.hpp"

namespace CrdImporter
{
class Point : public ElementBase
{
public:
   Point()           = default;
   ~Point() override = default;

   const std::string& GetLatString() const { return mLatString; }
   const std::string& GetLonString() const { return mLonString; }
   void               SetLatString(const std::string& aLat) { mLatString = aLat; }
   void               SetLonString(const std::string& aLon) { mLonString = aLon; }

   double GetLat() const { return mLat; }
   double GetLon() const { return mLon; }
   void   SetLat(double aLat) { mLat = aLat; }
   void   SetLon(double aLon) { mLon = aLon; }

private:
   std::string mLatString;
   std::string mLonString;
   double      mLat;
   double      mLon;
};
} // namespace CrdImporter
#endif
