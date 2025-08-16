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

#ifndef EXER_HPP
#define EXER_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "Set.hpp"

namespace usmtf
{
//! The EXER Set is a set that identifies a USMTF Message as an exercise, in contrast
//! to an OPER Set that identifies a USMTF as an operation

//! Example:
//! EXER/SOLID SHIELD 93/BLUE EYES//

//! USMTF Standard Excerpt:
//! EXER/                    /                                         //
//!     1 EXERCISE NICKNAME  2 EXERCISE MESSAGE ADDITIONAL IDENTIFIER

//! Detailed Document:
//! EXER.UF.RTF
class USMTF_EXPORT Exer : public Set
{
public:
   Exer(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const std::string& GetNickname() const;
   const std::string& GetExerciseMessageAdditionalIdentifier() const;
};
} // namespace usmtf

#endif
