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

#ifndef VALIDATABLE_HPP
#define VALIDATABLE_HPP

#include "usmtf_export.h"

#include "ErrorManager.hpp"

namespace usmtf
{
//! I wanted to unify the handling of Errors, validation of objects and error log exports.
//! A Validatable class has the methods for managing its validation and logging its errors.
class USMTF_EXPORT Validatable : public ErrorManager
{
public:
   bool IsValid() const noexcept;
};
} // namespace usmtf

#endif
