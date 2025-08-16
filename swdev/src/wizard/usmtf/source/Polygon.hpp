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

#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>
#include <vector>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Set.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{

/*!

The Polygon Set is used to represent a complex shape defined by coordinates.
     ______
    /      \
   /        \___
  |             |
  |             |
   \____________/

USMTF Example:
      POLYGON/LATM:2037N05943E/LATS:203632N0594256E/LATS:203932N0594256E/LATS:203932N0604256E/LATM:2037N05943E//


Standard Excerpt:
   POLYGON/          R         //
          1 POSITION OR POINT

Detailed Document:
POLYGON.UF.RTF
*/
class USMTF_EXPORT Polygon : public Set
{
public:
   Polygon(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   Polygon(const Polygon&) = delete;
   const std::vector<LatLon>& GetPoints() const noexcept;

private:
   bool                AreAllPointsValid();
   std::vector<LatLon> mPoints;
};
} // namespace usmtf

#endif
