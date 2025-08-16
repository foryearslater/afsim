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

#include "AOrbit.hpp"

#include "PositionUtils.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
AOrbit::AOrbit(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      mFirstOrbitPoint.ParseAndValidate(GetField(1));
      mSecondOrbitPoint.ParseAndValidate(GetField(2));
      mWidth.ParseAndValidate(GetField(3));
      mOrbitAlignment.ParseAndValidate(GetField(4));
      bool validationResult =
         mFirstOrbitPoint.IsValid() && mSecondOrbitPoint.IsValid() && mWidth.IsValid() && mOrbitAlignment.IsValid();
      if (!validationResult)
      {
         AddError("AORBIT Required Field Validations Failed", "AORBIT", "See Following Errors for details.");
         AddErrors(mFirstOrbitPoint.GetErrors());
         AddErrors(mSecondOrbitPoint.GetErrors());
         AddErrors(mWidth.GetErrors());
         AddErrors(mOrbitAlignment.GetErrors());
      }
   }
   catch (SetError&)
   {
      AddError("AOrbit Is Missing Required Fields", "AOrbit", "Please make sure AOrbit has fields in positions 1,2,3,4");
   }
}

const LatLon& AOrbit::GetFirstOrbitPoint() const noexcept
{
   return mFirstOrbitPoint;
}


const LatLon& AOrbit::GetSecondOrbitPoint() const noexcept
{
   return mSecondOrbitPoint;
}

const Radius& AOrbit::GetWidth() const noexcept
{
   return mWidth;
}

std::vector<UtLLPos> AOrbit::GetAlignmentOffsetPoints() const noexcept
{
   // This should probably move to members.
   auto originalCorners = FindSquareOfRefrencePositions(mFirstOrbitPoint.GetUtLLPos(),
                                                        mSecondOrbitPoint.GetUtLLPos(),
                                                        mWidth.GetRadiusInMeters() / 2);

   if (mOrbitAlignment.GetOrbitAlignment() == "L")
   {
      UtLLPos newFirstOrbitPoint  = originalCorners[0];
      UtLLPos newSecondOrbitPoint = originalCorners[1];
      auto    newCorners =
         FindSquareOfRefrencePositions(newFirstOrbitPoint, newSecondOrbitPoint, mWidth.GetRadiusInMeters() / 2);
      newCorners.push_back(newFirstOrbitPoint);
      newCorners.push_back(newSecondOrbitPoint);
      return newCorners;
   }
   else if (mOrbitAlignment.GetOrbitAlignment() == "R")
   {
      UtLLPos newFirstOrbitPoint  = originalCorners[2];
      UtLLPos newSecondOrbitPoint = originalCorners.back();
      auto    newCorners =
         FindSquareOfRefrencePositions(newFirstOrbitPoint, newSecondOrbitPoint, mWidth.GetRadiusInMeters() / 2);
      newCorners.push_back(newFirstOrbitPoint);
      newCorners.push_back(newSecondOrbitPoint);
      return newCorners;
   }
   else // center.
   {
      originalCorners.push_back(mFirstOrbitPoint.GetUtLLPos());
      originalCorners.push_back(mSecondOrbitPoint.GetUtLLPos());
      return originalCorners;
   }
}
} // namespace usmtf
