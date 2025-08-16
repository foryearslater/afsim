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

#ifndef EXPORTABLE_HPP
#define EXPORTABLE_HPP

#include "usmtf_export.h"

#include "InputBlock.hpp"

namespace usmtf
{
//! This used to be a part of the Transformer API, but new needs for items to be exportable
//! without having a segment required a refactoring to make it that items could fulfill
//! the contract of being exported by the script builder. So, in other words,
//! If you implement the exportable interface, your entities can be written out by the
//! ScriptBuilder.
class Exportable
{
public:
   virtual InputBlock GetInputBlock() const = 0;
   virtual ~Exportable()                    = default;
};
} // namespace usmtf
#endif
