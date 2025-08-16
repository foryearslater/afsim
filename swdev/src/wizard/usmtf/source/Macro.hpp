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

#ifndef MACRO_HPP
#define MACRO_HPP

#include "Exportable.hpp"

namespace usmtf
{
//! A Macro is just a collection of
//! data that is intended to be filled in by the user.
class Macro : public Exportable
{
public:
   explicit Macro(const std::string& aKey, const std::string& aInputScaffold) noexcept;
   InputBlock GetInputBlock() const override;
   //! used for indexing into collection of these to avoid duplicates. It doesnt
   //! make sense to have two macros with the same name in a single export.
   const std::string& GetKey() const noexcept;
   const std::string  GetPlaceholder() const noexcept;

private:
   const std::string mKey;
   const std::string mInputScaffold;
};
} // namespace usmtf
#endif
