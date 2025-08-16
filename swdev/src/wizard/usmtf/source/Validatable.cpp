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

#include "Validatable.hpp"

namespace usmtf
{
bool Validatable::IsValid() const noexcept
{
   // This will be how its done once its used.
   // return HasErrors();
   return !HasErrors();
}
} // namespace usmtf
