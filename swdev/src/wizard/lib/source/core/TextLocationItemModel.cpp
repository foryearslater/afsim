// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "TextLocationItemModel.hpp"

#include <QTextDocument>

#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"

QVariant wizard::ItemTranslateTextRange::TranslateLocation(const UtTextDocumentRange& aRange, int aRole) const
{
   QVariant rval(QVariant::Invalid);
   switch (aRole)
   {
   case Qt::DisplayRole:
      if ((aRange.mSource != nullptr) && !aRange.mSource->GetLines().Empty())
      {
         size_t  line     = aRange.mSource->PositionToLineNumber(aRange.GetBegin());
         QString fileName = QString::fromStdString(UtPath(aRange.mSource->GetFilePath()).GetFileName());
         rval             = fileName + " : " + QString::number(line + 1);
      }
      break;
   default:
      break;
   }
   return rval;
}

QVariant wizard::ItemTranslateTextRange::TranslateText(const UtTextDocumentRange& aRange, int aRole) const
{
   QVariant rval(QVariant::Invalid);
   switch (aRole)
   {
   case Qt::DisplayRole:
      if ((aRange.mSource != nullptr) && !aRange.mSource->GetLines().Empty())
      {
         size_t  line        = aRange.mSource->PositionToLineNumber(aRange.GetBegin());
         QString lineText    = QString::fromStdString(aRange.mSource->GetLineString(line));
         size_t  startOffset = aRange.GetBegin() < aRange.mSource->GetLinePosition(line) ?
                                  0 :
                                  aRange.GetBegin() - aRange.mSource->GetLinePosition(line);
         size_t  endOffset   = std::min(startOffset + aRange.Length(), ut::cast_to_size_t(lineText.size()));

         if (startOffset < endOffset)
         {
            lineText.insert(ut::cast_to_int(startOffset), "\n");
            lineText.insert(ut::cast_to_int(endOffset) + 1, "\r");
            lineText = lineText.toHtmlEscaped();
            lineText.replace('\n', "<b>");
            lineText.replace('\r', "</b>");
         }
         rval = lineText;
      }
      break;
   default:
      break;
   }
   return rval;
}

QVariant wizard::TextLocationItemModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
   int r = index.row();
   if (r >= (int)mLocations.size())
   {
      return QVariant::Invalid;
   }
   QVariant rval(QVariant::Invalid);
   if (index.column() == 0)
   {
      rval = mTextRangeToItem.TranslateLocation(mLocations[r], role);
   }
   else if (index.column() == 1 && mShowText)
   {
      rval = mTextRangeToItem.TranslateText(mLocations[r], role);
   }
   return rval;
}

QVariant wizard::TextLocationItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (orientation == Qt::Horizontal)
   {
      if (role == Qt::DisplayRole)
      {
         if (section == 0)
         {
            return tr("Location");
         }
         else if (section == 1)
         {
            return tr("Text");
         }
      }
   }
   return QVariant();
}

void wizard::TextLocationItemModel::SourceCleanupEligibility(QSet<TextSource*>& aEligibleSources)
{
   for (auto&& loc : mLocations)
   {
      TextSource* src = static_cast<TextSource*>(loc.mSource);
      aEligibleSources.remove(src);
   }
}

wizard::TextLocationItemModel::TextLocationItemModel(const std::vector<UtTextDocumentAutoUpdateRange>& aLocationList)
   : mShowText(false)
   , mLocations(aLocationList)
{
   connect(wizSignals, &Signals::SourceCleanupEligibility, this, &TextLocationItemModel::SourceCleanupEligibility);
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ProjectClosed, this, &TextLocationItemModel::ProjectClosed);
}

wizard::TextLocationItemModel::TextLocationItemModel(const std::vector<UtTextDocumentRange>& aLocationList)
   : mShowText(false)
{
   mLocations.clear();
   for (auto&& it : aLocationList)
   {
      mLocations.emplace_back(UtTextDocumentAutoUpdateRange(it));
   }

   connect(wizSignals, &Signals::SourceCleanupEligibility, this, &TextLocationItemModel::SourceCleanupEligibility);
   connect(ProjectWorkspace::Instance(), &ProjectWorkspace::ProjectClosed, this, &TextLocationItemModel::ProjectClosed);
}

void wizard::TextLocationItemModel::ProjectClosed(Project*)
{
   if (!mLocations.empty())
   {
      beginRemoveRows(QModelIndex(), 0, static_cast<int>(mLocations.size()) - 1);
      endRemoveRows();
      mLocations.clear();
   }
}

const std::vector<UtTextDocumentAutoUpdateRange>& wizard::TextLocationItemModel::GetLocations() const
{
   return mLocations;
}
