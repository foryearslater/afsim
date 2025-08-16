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

#ifndef AORBIT_HPP
#define AORBIT_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "LatLon.hpp"
#include "OrbitAlignment.hpp"
#include "Radius.hpp"
#include "Set.hpp"
#include "UtLLPos.hpp"
#include "Validatable.hpp"

namespace usmtf
{
//! The AOrbit Set is used to outline the positional details of a volume space that is shaped like a pill.
//! It consists of two circle and a rectangle polygon.
//!  __  _____________ __     //
//! /\ \              /\ \    //
//! \_\/ _____________\_\/    //

//! USMTF Example:
//! AORBIT/LATS:251538N0342044E/LATS:305942N0343122E/40NM/C//

//! Standard Excerpt:
//! AORBIT/                        /                        /                                                   / //
//!       1 FIRST POINT FOR ORBIT  2 SECOND POINT FOR ORBIT 3 WIDTH IN NAUTICAL MILES, METERS, OR KILOMETERS    4 ORBIT ALIGNMENT

//! Detailed Document:
//! AORBIT.UF.RTF
class USMTF_EXPORT AOrbit : public Set
{
public:
   AOrbit(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const LatLon& GetFirstOrbitPoint() const noexcept;
   const LatLon& GetSecondOrbitPoint() const noexcept;
   /// The width is the width of the polygon portion of the shape
   const Radius& GetWidth() const noexcept;
   /// Calculates the polygonal shape positions as well the offset circle centers based on alignment
   std::vector<UtLLPos> GetAlignmentOffsetPoints() const noexcept;

private:
   LatLon         mFirstOrbitPoint;
   LatLon         mSecondOrbitPoint;
   Radius         mWidth;
   OrbitAlignment mOrbitAlignment;
};
} // namespace usmtf

#endif
