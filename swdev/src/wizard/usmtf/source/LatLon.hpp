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

#ifndef LATLON_HPP
#define LATLON_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a LatLon.
//! A LatLon is a field that indicates latitude and longitude. It comes in
//! several forms.

//! USMTF Excerpt:
//! A.POSITION OR POINT, LAT / LONG, MINUTES
/*! LATITUDE, DEGREES	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 90).

   LATITUDE MINUTE(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LATITUDINAL HEMISPHERE	1 A
   USE TABLE A FOR ALLOWABLE ENTRIES

   LONGITUDE, DEGREES	3 N
   ALLOWABLE ENTRIES : (000 THROUGH 180).

   LONGITUDE MINUTE(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LONGITUDINAL HEMISPHERE	1 A
   USE TABLE B FOR ALLOWABLE ENTRIES

   EXAMPLE : / 3510N07901W*/


//! B.POSITION OR ROUTE POINT, LAT / LONG, SECONDS

/*! EXPRESSED AS FOLLOWS :
   LATITUDE, DEGREES	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 90).

   LATITUDE MINUTE(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LATITUDE SECOND(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LATITUDINAL HEMISPHERE	1 A
   USE TABLE A FOR ALLOWABLE ENTRIES

   LONGITUDE, DEGREES	3 N
   ALLOWABLE ENTRIES : (000 THROUGH 180).

   LONGITUDE MINUTE(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LONGITUDE SECOND(ANGULAR)	2 N
   ALLOWABLE ENTRIES : (00 THROUGH 59).

   LONGITUDINAL HEMISPHERE	1 A
   EXAMPLE : / 351025N0790125W*/
class USMTF_EXPORT LatLon : public FFIRN_Validator
{
public:
   LatLon() = default;
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
   void        SetUtLLPos() noexcept;
   bool        IsValidLatLonMinutes() noexcept;
   bool        IsValidLatLonSeconds() noexcept;
   bool        IsValidLattitudinalDegree(const std::string& aValue) noexcept;
   bool        IsValidLongitudinalDegree(const std::string& aValue) noexcept;
   bool        IsValidMinuteOrSecond(const std::string& aValue) noexcept;
   bool        IsValidLattitudinalHemisphere(const std::string& aValue) noexcept;
   bool        IsValidLongitudinalHemisphere(const std::string& aValue) noexcept;
   std::string mLattitudinalDegree;
   std::string mLongitudinalDegree;
   std::string mLatMinutes;
   std::string mLonMinutes;
   std::string mLatSeconds;
   std::string mLonSeconds;
   std::string mLatHemisphere;
   std::string mLonHemisphere;
   UtLLPos     mUtLLPos;
};
} // namespace usmtf
#endif
