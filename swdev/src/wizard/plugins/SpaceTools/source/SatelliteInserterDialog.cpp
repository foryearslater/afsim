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

#include "SatelliteInserterDialog.hpp"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include "InserterWidget.hpp"
#include "Signals.hpp"
#include "StartTimeWidget.hpp"
#include "TLE_Widget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

SpaceTools::SatelliteInserterDialog::SatelliteInserterDialog(QWidget* aParentPtr, PrefObject* aPrefObjectPtr)
   : QDialog(aParentPtr)
   , mInserterWidgetPtr(new InserterWidget(this, aPrefObjectPtr))
   , mTLE_WidgetPtr(new TLE_Widget(this, aPrefObjectPtr))
   , mStartTimeWidgetPtr(new StartTimeWidget(this))
{
   mUI.setupUi(this);

   mUI.mTabWidget->addTab(mInserterWidgetPtr, "Insert Satellites");
   mUI.mTabWidget->addTab(mTLE_WidgetPtr, "Update TLEs");
   mUI.mTabWidget->addTab(mStartTimeWidgetPtr, "Edit Start Time");

   connect(mUI.mCancelButton, &QPushButton::clicked, this, [&]() { close(); });
   connect(wizSignals, &wizard::Signals::EditorContextMenu, this, &SatelliteInserterDialog::EditorContextMenu);
   connect(mUI.mTabWidget, &QTabWidget::currentChanged, this, &SatelliteInserterDialog::Resize);
   connect(mUI.mShowPreferencesButton,
           &QPushButton::clicked,
           this,
           []() { wkfEnv.GetMainWindow()->ShowPreferencePage("Space Tools"); });
   resize(QSize(1500, 800));
}

//! Shows the Satellite Inserter dialog and updates all tabs
void SpaceTools::SatelliteInserterDialog::Show()
{
   mUI.mTabWidget->setCurrentWidget(mInserterWidgetPtr);
   mInserterWidgetPtr->OnShow();
   mStartTimeWidgetPtr->OnShow();
   mTLE_WidgetPtr->OnShow();
   show();
}

//! Resets the insert file on project opened
void SpaceTools::SatelliteInserterDialog::OnProjectOpened()
{
   mInserterWidgetPtr->SetInsertFile("", true);
}

void SpaceTools::SatelliteInserterDialog::OnProjectClosed()
{
   close();
}

//! Adds a context menu option to editors
void SpaceTools::SatelliteInserterDialog::EditorContextMenu(wizard::Editor* aEditorPtr, QMenu& aMenu)
{
   if (aEditorPtr)
   {
      mInserterWidgetPtr->SetInsertFile(QString::fromStdString(aEditorPtr->GetSource()->GetFilePath().GetNormalizedPath()));
      connect(aMenu.addAction(QIcon::fromTheme("satellite"), "Insert Satellites"),
              &QAction::triggered,
              this,
              &SatelliteInserterDialog::Show);
   }
}

void SpaceTools::SatelliteInserterDialog::Resize()
{
   if (mUI.mTabWidget->currentWidget() == mInserterWidgetPtr)
   {
      resize(QSize(1500, 800));
   }
   else
   {
      resize(minimumWidth(), minimumHeight());
   }
}
