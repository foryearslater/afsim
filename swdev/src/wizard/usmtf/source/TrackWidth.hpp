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

#ifndef TRACKWIDTH_HPP
#define TRACKWIDTH_HPP

#include "usmtf_export.h"

#include <memory>
#include <string>

#include "FFIRN_Validator.hpp"
#include "Radius.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a TrackWidth.
//! Which is simply 2 Radius Validators, see Radius.hpp
//! USMTF Example:
//!                                               TrackWidth
//!                                               ||||||||||
//! 1TRACK                                         vvvvvvvvvv
//!   / LEG / LEG - BEGIN     / LEG - END      / LEG - WIDTH / MINALT - MAXALT
//!   / 01 / 152345N0505657E / 192646N0531226E / 30.5NML - 60.9NMR / 050AMSL - 100AMSL
//!   / 02 / 192646N0531226E / 231031N0545323E / 60.5NML - 60.5NMR / 080AMSL - 120AMSL
//!   / 03 / 231031N0545323E / 280628N0562901E / 60NML - 80NMR / 100AMSL - 150AMSL
//!   //

class USMTF_EXPORT TrackWidth : public FFIRN_Validator
{
public:
   TrackWidth() = default;
   const Radius* GetLeftRadius() const noexcept;
   const Radius* GetRightRadius() const noexcept;
   bool          ParseAndValidate(const Field& aField) override;

protected:
   bool                    IsValidTrackWidth() noexcept;
   std::unique_ptr<Radius> mLeftRadius;
   std::unique_ptr<Radius> mRightRadius;
};
} // namespace usmtf
#endif
