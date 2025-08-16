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

#include "AircraftControlAgency.hpp"

namespace usmtf
{
AircraftControlAgency::AircraftControlAgency()
{
   // reference table in header file.
   std::string agencies = "(AOC|ASOC|ABC|AWAC|ASCA|CCT|CAOC|CRC|CRE|CRP|DASC|DASA|EWC|FACP|FAC|FACA|GCI|HST|ITG|JAOC|"
                          "MATC|MMT|OTR|TACC|TACP|TACA|TADC|TAOC)";
   std::string expr     = cStart + agencies + cEnd;
   SetRegexExpression(expr);
}

bool AircraftControlAgency::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mAgency = mMatchResults.captured(1).toStdString();
   return true;
}

const std::string& AircraftControlAgency::GetAgency() const noexcept
{
   return mAgency;
}

} // namespace usmtf
