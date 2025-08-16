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

#include "Polyarc.hpp"

#include "FFIRN_Validator.hpp"
#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Polyarc::Polyarc(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   try
   {
      if (!IsRadarcAndPolygonValid())
      {
         AddError("POLYARC Required Field Validations Failed", "POLYARC", "See Above Errors for details.");
      }
   }
   catch (SetError&)
   {
      AddError("POLYARC Is Missing Required Fields", "POLYARC", "Please make sure POLYARC has fields in required positions");
   }
}

const Radarc* Polyarc::GetRadarc() const noexcept
{
   return mRadarc.get();
}

const Polygon* Polyarc::GetPolygon() const noexcept
{
   return mPolygon.get();
}

bool Polyarc::IsRadarcAndPolygonValid()
{
   std::string        stf = "RADARC";
   std::vector<Field> fields{Field{stf}, GetField(1), GetField(2), GetField(4), Field{"0NM"}, GetField(3)};
   mRadarc = ut::make_unique<Radarc>(stf, fields);

   stf = "POLYGON";
   std::vector<Field> polygonFields{Field{stf}};
   auto               polygonStart = mFields.begin() + 5;
   polygonFields.insert(polygonFields.end(), polygonStart, mFields.end());
   mPolygon = ut::make_unique<Polygon>(stf, polygonFields);

   if (!mRadarc->IsValid())
   {
      AddError("Could Not Infer Radarc From Polyarc fields", "POLYARC", "See Below Error for Details");
      AddErrors(mRadarc->GetErrors());
      return false;
   }
   if (!mPolygon->IsValid())
   {
      AddError("Could Not Infer Polygon From Polyarc fields", "POLYARC", "See Below Error for Details");
      AddErrors(mPolygon->GetErrors());
      return false;
   }
   return true;
}

} // namespace usmtf
