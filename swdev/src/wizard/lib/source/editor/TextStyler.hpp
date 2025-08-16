// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#ifndef TEXTSTYLER_HPP
#define TEXTSTYLER_HPP

#include <QTextCharFormat>

namespace wizard
{
class TextStyler
{
public:
   virtual void SetTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt)   = 0;
   virtual void MergeTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt) = 0;
};
} // namespace wizard

#endif
