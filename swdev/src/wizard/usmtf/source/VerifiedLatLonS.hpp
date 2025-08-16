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

#ifndef VERIFIEDLATLONS_HPP
#define VERIFIEDLATLONS_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"
#include "UtLLPos.hpp"

// TODO Because this took the new route of Regex, i need to figure out how to consolidate base classes
// for latlon member re-use.
namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a VerifiedLatLonS.
//! A VerifiedLatLonS is a field that indicates latitude and longitude.
//! USMTF Excerpt:
//! / VLATS:351025N6-0790125W4
class USMTF_EXPORT VerifiedLatLonS : public RegexValidator
{
public:
   VerifiedLatLonS();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetLatDegree() const noexcept;
   const std::string& GetLonDegree() const noexcept;
   const std::string& GetLatSeconds() const noexcept;
   const std::string& GetLonSeconds() const noexcept;
   const std::string& GetLatMinutes() const noexcept;
   const std::string& GetLonMinutes() const noexcept;
   const std::string& GetLatHemisphere() const noexcept;
   const std::string& GetLonHemisphere() const noexcept;
   //! UtLLPos are a most useful entity in the AFSIM framework.
   //! Instead of reinventing the wheel in LatLon, we make an attempt
   //! to use UtLLPos when they fulfill our need.
   const UtLLPos&     GetUtLLPos() const noexcept;
   std::string        GetFormattedLatLon() const noexcept;
   static std::string GetFormattedLatLon(const UtLLPos& position) noexcept;

protected:
   virtual void SetUtLLPos() noexcept;
   std::string  mLattitudinalDegree;
   std::string  mLongitudinalDegree;
   std::string  mLatMinutes;
   std::string  mLonMinutes;
   std::string  mLatSeconds;
   std::string  mLonSeconds;
   std::string  mLatHemisphere;
   std::string  mLonHemisphere;
   UtLLPos      mUtLLPos;
};
} // namespace usmtf
#endif
