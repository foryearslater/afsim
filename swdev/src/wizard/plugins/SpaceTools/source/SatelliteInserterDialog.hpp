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

#ifndef SATELLITEINSERTERTABLEDIALOG_HPP
#define SATELLITEINSERTERTABLEDIALOG_HPP

#include <QDialog>

#include "Editor.hpp"
#include "InserterWidget.hpp"
#include "SatelliteInserterPrefObject.hpp"
#include "StartTimeWidget.hpp"
#include "TLE_Widget.hpp"
#include "ui_SatelliteInserterDialog.h"

namespace SpaceTools
{
//!
class SatelliteInserterDialog : public QDialog
{
public:
   SatelliteInserterDialog(QWidget* aParentPtr, PrefObject* aPrefObjectPtr);
   ~SatelliteInserterDialog() override = default;

   void Show();
   void OnProjectOpened();
   void OnProjectClosed();

   void EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu);

private:
   void Resize();
   //! The UI for the Satellite Inserter
   Ui::SatelliteInserter mUI;

   //! The widget that inserts satellites
   InserterWidget* mInserterWidgetPtr;

   //! The widget that updates TLEs
   TLE_Widget* mTLE_WidgetPtr;

   //! The widget that updates the start time/date/epoch
   StartTimeWidget* mStartTimeWidgetPtr;
};
} // namespace SpaceTools
#endif
