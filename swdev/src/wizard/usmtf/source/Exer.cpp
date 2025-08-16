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

#include "Exer.hpp"

#include "UtMemory.hpp"

namespace usmtf
{
Exer::Exer(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
}

const std::string& Exer::GetNickname() const
{
   return GetFieldContent(1);
}

const std::string& Exer::GetExerciseMessageAdditionalIdentifier() const
{
   return GetFieldContent(2);
}
} // namespace usmtf
