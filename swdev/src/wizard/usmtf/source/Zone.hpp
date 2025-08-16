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

#ifndef ZONE_HPP
#define ZONE_HPP

#include "APeriod.hpp"
#include "Acmid.hpp"
#include "Ampn.hpp"
#include "Efflevel.hpp"
#include "Message.hpp"
#include "Radarc.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Transformer.hpp"

namespace usmtf
{
//! A Zone is a USMTF Transformer that represents
//! the creation of an AFSIM zone from Segment blocks in an
//! Air Control Message. See any of the derived classes for details.
class Zone : public USMTF_Transformer
{
public:
   explicit Zone(const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;
   void       AddAmplificationBlockIfPresent(InputBlock& aInputBlock) const;

private:
   const Ampn* mOptionalAmpn;
};
} // namespace usmtf
#endif
