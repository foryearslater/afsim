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
#ifndef SATELLITEINSERTERPREFWIDGET_HPP
#define SATELLITEINSERTERPREFWIDGET_HPP

#include <QListWidget>
#include <QString>
#include <QWidget>

#include "SatelliteInserterPrefObject.hpp"
#include "WkfPrefWidget.hpp"
#include "ui_SatelliteInserterPrefWidget.h"

namespace SpaceTools
{
//! The preference widget for the Satellite Inserter.
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget();
   ~PrefWidget() override = default;

   void UpdateDatabases();

private:
   void    ReadPreferenceData(const PrefData& aPrefData) override;
   void    UpdateCheckState(QListWidgetItem* aItemPtr);
   void    AddDatabase();
   void    CheckAllDatabases();
   void    SetDatabaseCheckAll();
   void    DeleteLoadedDatabases();
   void    DatabaseSelectionChanged();
   void    WritePreferenceData(PrefData& aPrefData) override;
   QString GetDisplayName(const QString& aPath) const;

   //! The UI for the preference widget
   Ui::PrefWidget mUI;
};

//! The widget that tracks the current and saved filters.
class FilterListWidget : public QListWidget
{
public:
   FilterListWidget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr);

private:
   void contextMenuEvent(QContextMenuEvent* aEventPtr) override;

   PrefObject* mPrefObjectPtr;
};
} // namespace SpaceTools

#endif // SATELLITEINSERTERPREFWIDGET_HPP
