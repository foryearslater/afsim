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
#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include "usmtf_export.h"

#include "Set.hpp"

namespace usmtf
{
//! A segment is an extracted aggregation of Sets from a message, usually indicating some
//! logical idea. Transforms will take segments and create script blocks out of them
//! The reason they are their own class is for some convenience while working with them.
//! They are similar to a Message in that they are a collection of sets with some similar convenience
//! methods, however they are distinct in that they are just an arbitrary collection of sets, not a
//! USMTF governed collection.
//! Simply think of them as an excerpt from a Message.
//! Segment is actual USMTF Parlance or id have called this message excerpt.
//! Segments are the paragraph of the USMTF if you will.
class USMTF_EXPORT Segment
{
public:
   /// Location in file, useful for logging errors.
   using SetPosition  = size_t;
   using SetLocation  = std::pair<const Set&, SetPosition>;
   using SetLocations = std::vector<SetLocation>;
   explicit Segment(const SetLocations& aSets) noexcept;
   const SetLocation         GetSet(const std::string& aSTF) const;
   const Set&                GetSet(SetPosition aSetIndex) const;
   std::vector<SetLocations> GetSubSegments(const std::string& aSTF) const;
   SetLocations              GetSubSegment(const SetPosition& startPosition, const SetPosition& stopPosition) const;
   SetLocations              GetSets(const std::string& aSTF) const noexcept;
   size_t                    GetNumberOfSets() const noexcept;
   bool                      HasSet(const std::string& aSTF) const;
   /// Iteration support
   SetLocations::const_iterator begin() const;
   SetLocations::const_iterator end() const;

private:
   bool               IsSetPositionOutOfBounds(const SetPosition& aPos) const noexcept;
   const SetLocations mSets;
};
} // namespace usmtf

#endif
