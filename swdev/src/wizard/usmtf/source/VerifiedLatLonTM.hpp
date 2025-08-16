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

#ifndef VERIFIEDLATLONTM_HPP
#define VERIFIEDLATLONTM_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "VerifiedLatLonS.hpp"

//! Ideally we shouldn't need so many specialization of VerifiedLatLon to get the right validation and member
//! extraction. Unfortunately I don't have the time right now to figure that out for general use. Come back to this once
//! ACO scaffold is done to repair it. I suspect it will be some method of dynamic regex group selection for the minute
//! seconds fields.

//! There was a divergence of LatLon from manual parsing to RegexParsing, and now the specializations for precision.
//! these solutions are history of learning new use cases and api improvement, but, thats no excuse, they need
//! consolidated when time allows. 2/11/2020.

//! this does not apply to non verified LatLons, as they have  a completely different structure and parsing needs.
//! However, this can probably be addressed as well. Im certain we can find common ground and some solutions to dry this
//! all up and simplify it. Example: VGEOK:3510.234N8-07901.123W3
namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a VerifiedLatLonTM.
//! Same as other lat long just a more precise representation of the minutes field in thousands of a minute.
class USMTF_EXPORT VerifiedLatLonTM : public VerifiedLatLonS
{
public:
   VerifiedLatLonTM();
   bool ParseAndValidate(const Field& aField) override;
   // VerifiedLatLonS members

protected:
   // VerifiedLatLonS members
   std::string CalculateSeconds(std::string& aMinuteFraction, double& secondFraction) noexcept;
   void        SetUtLLPos() noexcept override;
   double      mLatSecondsFraction;
   double      mLonSecondsFraction;
};
} // namespace usmtf
#endif
