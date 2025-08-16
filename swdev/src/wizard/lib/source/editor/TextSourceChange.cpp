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

#include "TextSourceChange.hpp"

void wizard::TextSourceChange::ApplyChangeToRange(UtTextRange& aRange) const
{
   if (mPos > aRange.GetEnd())
   {
      return;
   }
   if (mCharsRemoved > 0)
   {
      if (mPos < aRange.GetBegin()) // move range
      {
         aRange.SetBegin(std::max(mPos, aRange.GetBegin() - mCharsRemoved));
      }
      aRange.SetEnd(std::max(mPos, aRange.GetEnd() - mCharsRemoved)); // shrink range
   }
   else
   {
      if (mPos < aRange.GetBegin()) // move range
      {
         aRange.SetBegin(aRange.GetBegin() + mText.size());
      }
      aRange.SetEnd(aRange.GetEnd() + mText.size()); // grow range
   }
}

size_t wizard::TextSourceChange::ApplyChangeToPos(size_t aPos) const
{
   if (mPos > aPos)
   {
      return aPos;
   }
   if (mCharsRemoved > 0)
   {
      if (mPos < aPos) // move range
      {
         aPos = std::max(mPos, aPos - mCharsRemoved);
      }
   }
   else
   {
      if (mPos < aPos)
      {
         aPos += mText.size();
      }
   }
   return aPos;
}

void wizard::TextSourceChange::ApplyToPartialDocument(UtTextDocument& aDocument, UtTextRange& aSection) const
{
   if (mPos > aSection.GetEnd())
   {
      return;
   }
   if (mCharsRemoved > 0)
   {
      if (mPos < aSection.GetBegin()) // move range
      {
         size_t newBegin = aSection.GetBegin() - mCharsRemoved;
         if (newBegin >= mPos) // delete is all before
         {
            aSection.SetBegin(newBegin);
         }
         else // delete the first part of the range
         {
            aDocument.Erase(0, std::min(mPos - newBegin, aDocument.GetText().Size()));
            aSection.SetBegin(mPos);
         }
         aSection.SetEnd(std::max(mPos, aSection.GetEnd() - mCharsRemoved)); // shrink range
      }
      else
      {
         size_t delPos       = mPos - aSection.GetBegin();
         size_t charsDeleted = std::min(aSection.GetEnd() - delPos, mCharsRemoved);
         aDocument.Erase(delPos, charsDeleted);
         aSection.SetEnd(aSection.GetEnd() - charsDeleted);
      }
   }
   else
   {
      if (mPos < aSection.GetBegin()) // move range
      {
         aSection.SetBegin(aSection.GetBegin() + mText.size());
      }
      aDocument.Insert(mPos - aSection.GetBegin(), mText.c_str(), mText.size());
      aSection.SetEnd(aSection.GetEnd() + mText.size()); // grow range
   }
}

void wizard::TextSourceChange::ApplyToDocument(UtTextDocument& aDocument) const
{
   if (mCharsRemoved > 0)
   {
      size_t docSize = aDocument.GetText().Size();
      if (!docSize)
         return;
      size_t removeCount = mCharsRemoved;
      if (docSize <= mPos + mCharsRemoved)
      {
         removeCount = docSize - mPos;
         // There should be a trailing null byte, don't remove it
         if (aDocument.GetText()[docSize - 1] == 0)
         {
            --removeCount;
         }
      }
      aDocument.Erase(mPos, removeCount);
   }
   else
   {
      assert(mText.find('\0') == std::string::npos);
      aDocument.Insert(mPos, mText.c_str(), (int)mText.size());
   }
}

void wizard::ApplyToDocument(UtTextDocument& aDocument, const QVector<TextSourceChange>& aChanges)
{
   for (int i = 0; i < aChanges.size(); ++i)
   {
      aChanges[i].ApplyToDocument(aDocument);
   }
}

bool wizard::TextSourceChange::operator==(const TextSourceChange& aRhs)
{
   return (mText == aRhs.mText) && (mPos == aRhs.mPos) && (mCharsRemoved == aRhs.mCharsRemoved);
}

QDataStream& operator<<(QDataStream& s, const wizard::TextSourceChange& d)
{
   s << QString::fromStdString(d.mText) << static_cast<quint64>(d.mPos) << static_cast<quint64>(d.mCharsRemoved)
     << d.mSequenceNumber;
   return s;
}

QDataStream& operator>>(QDataStream& s, wizard::TextSourceChange& d)
{
   QString txt;
   quint64 pos;
   quint64 charsRemoved;
   s >> txt >> pos >> charsRemoved >> d.mSequenceNumber;
   d.mPos          = pos;
   d.mCharsRemoved = charsRemoved;
   d.mText         = txt.toStdString();
   return s;
}
