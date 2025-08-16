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

#include "UTM_Location.hpp"

#include <math.h>

#include "LatLon.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtLatPos.hpp"
#include "UtLonPos.hpp"

namespace usmtf
{

UTM_Location::UTM_Location()
{
   // Not really reusable anywhere else so not placing in RegexValidator
   const std::string zoneRow      = R"((C|D|E|F|G|H|J|K|L|M|N|P|Q|R|S|T|U|V|W|X))";
   const std::string squareColumn = R"((A|B|C|D|E|F|G|H|J|K|L|M|N|P|Q|R|S|T|U|V|W|X|Y|Z))";
   const std::string squareRow    = R"((A|B|C|D|E|F|G|H|J|K|L|M|N|P|Q|R|S|T|U|V))";
   std::string       expr = cStart + c01to60 + zoneRow + squareColumn + squareRow + c000to99999 + c000to99999 + cEnd;
   SetRegexExpression(expr);
}

bool UTM_Location::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mZoneColumn   = mMatchResults.captured(1).toStdString();
   mZoneRow      = mMatchResults.captured(2).toStdString();
   mSquareColumn = mMatchResults.captured(3).toStdString();
   mSquareRow    = mMatchResults.captured(4).toStdString();
   mEasting      = mMatchResults.captured(5).toStdString();
   mNorthing     = mMatchResults.captured(6).toStdString();
   return true;
}

const std::string& UTM_Location::GetZoneColumn() const noexcept
{
   return mZoneColumn;
}

const std::string& UTM_Location::GetZoneRow() const noexcept
{
   return mZoneRow;
}

const std::string& UTM_Location::GetSquareColumn() const noexcept
{
   return mSquareColumn;
}

const std::string& UTM_Location::GetSquareRow() const noexcept
{
   return mSquareRow;
}

const std::string& UTM_Location::GetEasting() const noexcept
{
   return mEasting;
}

const std::string& UTM_Location::GetNorthing() const noexcept
{
   return mNorthing;
}

int UTM_Location::GetEastingInMeters() const noexcept
{
   int casted;
   std::stringstream(mEasting) >> casted;
   if (mEasting.length() == 3)
   {
      return casted * 100;
   }
   else if (mEasting.length() == 4)
   {
      return casted * 10;
   }
   else
   {
      return casted;
   }
}

int UTM_Location::GetNorthingInMeters() const noexcept
{
   int casted;
   std::stringstream(mEasting) >> casted;
   if (mNorthing.length() == 3) // 100s of meters
   {
      return casted * 100;
   }
   else if (mNorthing.length() == 4) // 10s of meters
   {
      return casted * 10;
   }
   else // in meters
   {
      return casted;
   }
}

const std::string UTM_Location::GetFormattedLocation() const noexcept
{
   return LatLon::GetFormattedLatLon(ConvertToLLPos());
}

UtLLPos UTM_Location::ConvertToLLPos() const noexcept
{
   UtLLPos llPosition{};
   if (!IsValid())
   {
      return llPosition;
   }

   // Calculate latlong from UTM
   double latDegrees;
   double lonDegrees;
   if (UtEllipsoidalEarth::ConvertMGRSToLL(mField.GetContent(), latDegrees, lonDegrees))
   {
      UtLatPos afsimLatPos(latDegrees);
      UtLonPos afsimLonPos(lonDegrees);
      llPosition.Set(afsimLatPos, afsimLonPos);
   }
   return llPosition;
}
} // namespace usmtf
