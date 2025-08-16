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

#ifndef INSERTERWIDGET_HPP
#define INSERTERWIDGET_HPP

#include <map>
#include <vector>

#include <QDialog>
#include <QListWidget>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "SatelliteInserterHandler.hpp"
#include "SatelliteInserterPrefObject.hpp"
#include "UtMemory.hpp"
#include "ui_SatelliteInserterWidget.h"

namespace SpaceTools
{
//! Class for the Satellite Inserter Filter models
class FilterModel : public QSortFilterProxyModel
{
public:
   explicit FilterModel(QObject* aParentPtr, int aColumn, int aType, const QVariant& aValue);
   explicit FilterModel(QObject* aParentPtr);
   ~FilterModel() override = default;

   bool filterAcceptsRow(int aSourceRow, const QModelIndex& aSourceParent) const override;

   //! Column accessor
   int GetColumn() const { return mColumn; }

   //! Type accessor
   int GetType() const { return mType; }

   //! Value accessor
   QVariant GetValue() const { return mValue; }
   bool     lessThan(const QModelIndex& aLeftSource, const QModelIndex& aRightSource) const override;
   QVariant headerData(int aSection, Qt::Orientation aOrientation, int aRole) const override;

   //! Sets the filter type
   //!
   //! @param aState The new filtered type
   void SetTypeFiltered(bool aState) { mTypeFiltered = aState; }

   //! Sets the comparator type and value
   //!
   //! @param aComp The new comparator type
   //! @param aValue The new value
   void SetComparatorAndValue(const int aComp, const QVariant& aValue)
   {
      mType  = aComp;
      mValue = aValue;
      invalidateFilter();
   }

private:
   bool Compare(const QVariant& aLeft, int aCompType, const QVariant& aRight, unsigned int aPlatformIndex) const;

   int      mColumn;
   int      mType;
   QVariant mValue;
   bool     mTypeFiltered;
};

//!
class InserterWidget : public QWidget
{
public:
   InserterWidget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr);

   ~InserterWidget() override = default;

   enum FilterType
   {
      cUNSET,
      cEQUAL,
      cNOT_EQUAL,
      cCONTAINS
   };

   void ClearSelection();
   void SetModel(QAbstractItemModel* aModelPtr, bool aSetSource = false);
   void SetInsertFile(const QString& aFile = "", bool aReset = false);
   void UpdateTable();
   void Sort(int aColumn, Qt::SortOrder aSortOrder);
   void OnShow();

private:
   void InsertOptions();
   void InsertSatellites();

   // Filter Functions
   void                AddFilterRule(QListWidgetItem* aItemPtr);
   void                DisableInsertButton();
   void                ClearRules();
   void                CreateFilter(const QVariant& aData, int aColumn, int aFilterType = FilterType::cUNSET);
   void                EditRule(QListWidgetItem* aItemPtr);
   void                EnableInsert(const QItemSelection& aSelected, const QItemSelection& aDeselected);
   QDialog::DialogCode GetFilter(const int aColumn, int& aFilterType, QVariant& aValue);
   QString             GetRuleText(const int aColumn, const int aFilterType, const QVariant& aValue);
   QString             GetTypeText(int aFilterType);
   void                LoadFilter();
   void                LoadFilter(const std::vector<PrefData::Filter>& aFilter);
   void                RemoveRule(QListWidgetItem* aItemPtr);
   void                RuleContextMenu(const QPoint& aPoint);
   void                SaveFilter();
   void                TableItemContextMenu(const QPoint& aPoint);

   //! The list of current filter models
   std::map<int, FilterModel*> mFilterModels;

   //! The ID manager for filter IDs
   unsigned int mIdManager;

   //! The pointer to the preference object for the Satellite Inserter
   PrefObject* mPrefObjectPtr;

   //! The source model
   QAbstractItemModel* mSourceModelPtr;

   std::unique_ptr<SatelliteInserterHandler> mHandlerPtr;

   //! The UI for the Satellite Inserter
   Ui::InserterWidget mUI;
};
} // namespace SpaceTools
#endif
