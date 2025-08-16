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

#ifndef COMMENT_HPP
#define COMMENT_HPP

#include "USMTF_Transformer.hpp"

namespace usmtf
{
//! A comment transformer takes a segment
//! and makes a script block of the sets original form
//! as nothing but AFSIM comments
class Comment : public USMTF_Transformer
{
public:
   explicit Comment(const Segment& aSegment) noexcept;
   InputBlock GetInputBlock() const override;
};
} // namespace usmtf
#endif
