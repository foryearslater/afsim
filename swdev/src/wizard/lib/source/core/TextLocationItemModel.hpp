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
#ifndef TEXTLOCATIONITEMMODEL_HPP
#define TEXTLOCATIONITEMMODEL_HPP

#include <QAbstractListModel>
#include <QString>
#include <QVariant>
#include <UtTextDocument.hpp>

#include "ViExport.hpp"

namespace wizard
{
class Project;
class TextSource;

//! Handles converting a UtTextDocumentRange to items in Qt views
class ItemTranslateTextRange
{
public:
   QVariant TranslateLocation(const UtTextDocumentRange& aRange, int aRole) const;
   QVariant TranslateText(const UtTextDocumentRange& aRange, int aRole) const;
};

//! A model containing a list of UtTextDocumentRange's
//! Provides the first column as the file/line location
//! Optionally, provides the second column as the line text
class VI_EXPORT TextLocationItemModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   explicit TextLocationItemModel(const std::vector<UtTextDocumentAutoUpdateRange>& aLocationList);
   explicit TextLocationItemModel(const std::vector<UtTextDocumentRange>& aLocationList);
   int rowCount(const QModelIndex& parent /* = QModelIndex */) const override
   {
      if (parent.isValid())
      {
         return 0;
      } // no nesting
      return (int)mLocations.size();
   }
   int      columnCount(const QModelIndex& parent) const override { return mShowText ? 2 : 1; }
   QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

   const std::vector<UtTextDocumentAutoUpdateRange>& GetLocations() const;

   ItemTranslateTextRange mTextRangeToItem;
   //! When 'true', the model contains a second column containing the line text
   bool mShowText;

public slots:

   void SourceCleanupEligibility(QSet<TextSource*>& aEligibleSources);
   void ProjectClosed(Project*);

protected:
   std::vector<UtTextDocumentAutoUpdateRange> mLocations;
};
} // namespace wizard

#endif
