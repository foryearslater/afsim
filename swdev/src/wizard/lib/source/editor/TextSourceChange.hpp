// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TEXTSOURCECHANGE_HPP
#define TEXTSOURCECHANGE_HPP

#include <QDataStream>
#include <QVector>

#include "UtTextDocument.hpp"
#include "ViExport.hpp"

namespace wizard
{
struct VI_EXPORT TextSourceChange
{
   TextSourceChange()
      : mText()
      , mPos(0)
      , mCharsRemoved(0)
      , mSequenceNumber(-1)
   {
   }

   // Update a range of text after applying this change
   void   ApplyChangeToRange(UtTextRange& aRange) const;
   size_t ApplyChangeToPos(size_t aPos) const;
   void   ApplyToDocument(UtTextDocument& aDocument) const;

   // Applies the change to a document which represents only a section of the source document.
   void ApplyToPartialDocument(UtTextDocument& aDocument, UtTextRange& aSection) const;
   bool operator==(const TextSourceChange& aRhs);

   std::string mText;
   size_t      mPos;
   size_t      mCharsRemoved;
   //! Records the time the change was made
   long long mSequenceNumber;
};

// Apply a list of changes to a document
void ApplyToDocument(UtTextDocument& aDocument, const QVector<TextSourceChange>& aChanges);
} // namespace wizard
VI_EXPORT QDataStream& operator<<(QDataStream& s, const wizard::TextSourceChange& d);
VI_EXPORT QDataStream& operator>>(QDataStream& s, wizard::TextSourceChange& d);

#endif
