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


#include "SectorGeometry.hpp"


namespace usmtf
{
SectorGeometry::SectorGeometry()
   : mRange{}
{
   SetValidRange(0, 259);
}

bool SectorGeometry::ParseAndValidate(const Field& aField)
{
   // place holder
   //  Split pieces and delegate.
   std::string              fieldContent = aField.GetContent();
   std::vector<std::string> orientationTable{"T", "G", "R", "M"};
   std::string              orientation;
   CheckForAnyDelimeter(fieldContent, orientationTable, orientation);
   if (orientation.empty())
   {
      AddError("SectorGeometry Failed Parsing",
               fieldContent,
               "SectorGeometry must contain a delineation string of <T, M, R, G> that represent orientation.");
      return false;
   }

   try
   {
      auto        parts  = SplitOnDelimeter(fieldContent, orientation);
      std::string angles = parts.at(0);
      std::string range  = parts.at(1);


      if (!IsStringLength(angles, 6))
      {
         AddError("SectorGeometry Failed Parsing Angles",
                  angles,
                  "SectorGeometry must contain angle section of size 6,  example: 058190");
         return false;
      }

      // substring and validate angles
      std::string startAngle = angles.substr(0, 3);
      std::string stopAngle  = angles.substr(3);

      if (!IsInvalidRange(startAngle))
      {
         return false; // As usual Range validator adds the errors.
      }
      startAngle = std::to_string(mCastedRangeValue);

      if (!IsInvalidRange(stopAngle))
      {
         return false; // As usual Range validator adds the errors.
      }
      stopAngle = std::to_string(mCastedRangeValue);

      // delegate range validation
      mRange.ParseAndValidate(Field{range});
      if (!mRange.IsValid())
      {
         AddErrors(mRange.GetErrors());
         return false;
      }

      // I try to always defer setting the members to that the API for Validators is consistent.
      // an Invalid validator has defaults, not parts that were valid, parts that were not.
      mSectorStartAngle   = startAngle;
      mSectorStopAngle    = stopAngle;
      mAngularOrientation = orientation;
      return true;
   }
   catch (const std::out_of_range&)
   {
      AddError("SectorGeometry Failed Parsing",
               fieldContent,
               "SectorGeometry must contain three parts, angles, orientation and distance. Example: 058190T45-60NM");
      return false;
   }
   return true;
}

const std::string& SectorGeometry::GetSectorStartAngle() const noexcept
{
   return mSectorStartAngle;
}

const std::string& SectorGeometry::GetSectorStopAngle() const noexcept
{
   return mSectorStopAngle;
}

const std::string& SectorGeometry::GetAngleOrientation() const noexcept
{
   return mAngularOrientation;
}

const InnerOuterRange& SectorGeometry::GetRange() const noexcept
{
   return mRange;
}
} // namespace usmtf
