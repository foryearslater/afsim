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

#ifndef ELIPTICALAREA_HPP
#define ELIPTICALAREA_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an ElipticalArea
//! An EllipticalArea represents a space denoted by an ellipses.

//! Example:
//!  | Horizontal Axis
//!  |     | Vertical Axis
//!  |     |     |  Degree of rotation from True North, clockwise
//!  v     v     v
//! 105M-200M-240.0

//! The mathematical terms major and minor axis are not used here
//! but you can think of the smaller distance as the minor and the larger as the
//! major

class USMTF_EXPORT EllipticalArea : public FFIRN_Validator
{
public:
   EllipticalArea() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetHorizontalAxisDistance() const noexcept;
   const std::string& GetHorizontalAxisUOM() const noexcept;
   const std::string& GetVerticalAxisDistance() const noexcept;
   const std::string& GetVerticalAxisUOM() const noexcept;
   float              GetAngleFromTrueNorth() const noexcept;

protected:
   enum class Axis
   {
      Horizontal,
      Vertical
   };
   bool        IsValidEllipsesArea();
   bool        IsValidAxis(const std::string& aAxis, Axis axis);
   bool        IsValidDegree(const std::string& aDegree);
   std::string mHorizontalAxisDistance;
   std::string mHorizontalAxisUOM;
   std::string mVerticalAxisDistance;
   std::string mVerticalAxisUOM;
   float       mAngleInDegrees;
};
} // namespace usmtf
#endif
