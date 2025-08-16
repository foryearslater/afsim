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

#ifndef EFFLEVEL_HPP
#define EFFLEVEL_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "Field.hpp"
#include "Set.hpp"
#include "VerticalDimension.hpp"

namespace usmtf
{
//! The Efflevel Set is used to provide a lower and upper bound range that defines altitude space.
//! See VerticalDimension.hpp for details

//! Example:
//! EFFLEVEL/RAFL:070AMSL-FL280//

//! USMTF Standard Excerpt:
//! EFFLEVEL/                     //
//!          1 VERTICAL DIMENSION

//! Detailed Document:
//! EFFLEVEL.UF.RTF
class USMTF_EXPORT Efflevel : public Set
{
public:
   Efflevel(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const VerticalDimension& GetVerticalDimension() const noexcept;

private:
   VerticalDimension mVerticalDimension;
};
} // namespace usmtf

#endif
