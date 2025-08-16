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

#ifndef ORBITALALIGNMENT_HPP
#define ORBITALALIGNMENT_HPP

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an OrbitAlignment
//! An OrbitAlignment is a field that indicates the offset concerns from positions provided.

//!                                                    OrbitAlighment
//!                                                            |
//!                                                            v
//! EXAMPLE:	AORBIT/LATM:2037N05943E/LATS:203632N0594256E/15NM/C//

//! Valid Options are:
//! C - indicates center alignment, where corners are calculated equally left/right from provided point and width.
//! L - indicates left alignment, where right corners are the positions provided and left are calculated with width.
//! R - indicates right alignment, where left corners are the positions provided and right are calculated with width.
class OrbitAlignment : public FFIRN_Validator
{
public:
   OrbitAlignment() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetOrbitAlignment() const noexcept;

protected:
   bool        IsValidOrbitAlignment() noexcept;
   std::string mOrbitAlignment;
};
} // namespace usmtf
#endif
