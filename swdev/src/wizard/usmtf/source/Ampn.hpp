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

#ifndef AMPN_HPP
#define AMPN_HPP


#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The Amplification Set is used to provide additional details to augment a Segment or Set.
//! Example:
//!                  Amplification Text
//!                  | | | | |  | | | |
//!                  v v v v v  v v v v
//! AMPN/LTL BELOW 3000 FEET AGL, EXCEPT OVER WATER//

//! Detailed Document:
//! AMPN.UF.RTF
class USMTF_EXPORT Ampn : public Set
{
public:
   Ampn(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const std::string& GetAmplificationText() const;
};
} // namespace usmtf

#endif
