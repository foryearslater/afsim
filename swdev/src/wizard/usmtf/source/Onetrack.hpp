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

#ifndef ONETRACK_HPP
#define ONETRACK_HPP

#include "usmtf_export.h"

#include <string>
#include <tuple>

#include "Field.hpp"
#include "LatLon.hpp"
#include "Set.hpp"
#include "TabularSet.hpp"
#include "TrackWidth.hpp"
#include "UtLLPos.hpp"
#include "VerticalDimension.hpp"

namespace usmtf
{
/*!
 The OneTrack Set is used to provide the information of 2 or more polygon volumes that define a shape
similar to a corridor but with less edge accuracy. In other words, the polygon defined in the
set legs are used without corner interpolation.

    _____________               _____________
   |             | ___________ |             |
   |_____________||           ||_____________|
                  |___________|

 USMTF Example:
   1TRACK
   /LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
   / 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
   / 02/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
   / 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL


 USMTF Standard Excerpt:
   1TRACK
   /LEG         /LEG-BEGIN         /LEG-END       /LEG-WIDTH     /MINALT-MAXALT
   /            /                  /              /              /                     //
   1 TRACK LEG  2 BEGINNING POINT  3 ENDING POINT 4 TRACK WIDTH  5 VERTICAL DIMENSION


 Detailed Document:
 1TRACK.UF.RTF
*/
class USMTF_EXPORT OneTrack : public TabularSet
{
public:
   using Leg  = std::tuple<size_t, LatLon, LatLon, TrackWidth, VerticalDimension>;
   using Legs = std::vector<Leg>;
   OneTrack(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   OneTrack(const OneTrack&) = delete;
   const Legs& GetLegs() const noexcept;

private:
   bool AreLegsValid();
   bool AreLegsSequential(const std::string& aLegSequence, size_t aCurrentIteration);
   Legs mLegs;
};
} // namespace usmtf
#endif
