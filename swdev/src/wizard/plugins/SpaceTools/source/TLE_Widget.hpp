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

#ifndef TLEWIDGET_HPP
#define TLEWIDGET_HPP

#include <QWidget>

#include "SatelliteInserterHandler.hpp"
#include "SatelliteInserterPrefObject.hpp"
#include "ui_TLE_UpdaterWidget.h"

namespace SpaceTools
{
//!
class TLE_Widget : public QWidget
{
public:
   TLE_Widget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr);
   ~TLE_Widget() override = default;

   void OnShow();

private:
   void UpdateTLECheckState(QListWidgetItem* aItemPtr);
   void AddTLE_File();
   void CheckAllTLEs();
   void SetTLE_CheckAll();
   void RemoveTLE_Files();
   void TLE_SetSelectionChanged();
   void UpdateTLEs();
   void Initialize();
   void SaveTLE_Files();

   Ui::TLE_UpdaterWidget mUI;
   PrefObject*           mPrefObjectPtr;
   bool                  mIsInitialized{false};
};
} // namespace SpaceTools
#endif
