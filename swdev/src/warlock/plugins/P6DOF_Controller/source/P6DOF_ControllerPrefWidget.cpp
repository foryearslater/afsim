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

#include "P6DOF_ControllerPrefWidget.hpp"

#include <QComboBox>
#include <QHeaderView>
#include <QRadioButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "P6DOF_ControllerPrefObject.hpp"

WkP6DOF_Controller::PrefWidget::PrefWidget()
   : wkf::PrefWidgetT<PrefObject>()
{
   setWindowTitle("P6DOF Controller");

   QVBoxLayout* lo = new QVBoxLayout(this);
   lo->setObjectName(QStringLiteral("verticalLayout"));
   QTreeWidget*     vectorTree        = new QTreeWidget(this);
   QTreeWidgetItem* __qtreewidgetitem = new QTreeWidgetItem();
   __qtreewidgetitem->setText(0, QStringLiteral("1"));
   vectorTree->setHeaderItem(__qtreewidgetitem);
   vectorTree->setObjectName(QStringLiteral("vectorTree"));
   vectorTree->header()->setVisible(false);
   vectorTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
   lo->addWidget(vectorTree);
   vectorTree->setColumnCount(2);

   mShowHud = new QTreeWidgetItem(vectorTree->invisibleRootItem(), QStringList("Show HUD"));
   mShowHud->setCheckState(0, Qt::Unchecked);
   mShowHud->setExpanded(true);

   mFirstPerson = new QTreeWidgetItem(vectorTree->invisibleRootItem(), QStringList("1st Person View"));
   mFirstPerson->setCheckState(0, Qt::Unchecked);

   mHDD = new QTreeWidgetItem(vectorTree->invisibleRootItem(), QStringList("Launch HDD"));
   mHDD->setCheckState(0, Qt::Unchecked);

   QTreeWidgetItem* pilotMode =
      new QTreeWidgetItem(vectorTree->invisibleRootItem(), QStringList("Pilot Mode (applied to new windows)"));
   QTreeWidgetItem* autopilotItem = new QTreeWidgetItem(pilotMode, QStringList("    Autopilot"));
   mAutopilot                     = new QRadioButton(this);
   vectorTree->setItemWidget(autopilotItem, 0, mAutopilot);
   mAutopilot->setChecked(false);

   QTreeWidgetItem* manualPilotItem = new QTreeWidgetItem(pilotMode, QStringList("    Manual Pilot"));
   mManualPilot                     = new QRadioButton(this);
   vectorTree->setItemWidget(manualPilotItem, 0, mManualPilot);
   mManualPilot->setChecked(false);

   QTreeWidgetItem* syntheticPilotItem = new QTreeWidgetItem(pilotMode, QStringList("    Synthetic Pilot"));
   mSyntheticPilot                     = new QRadioButton(this);
   vectorTree->setItemWidget(syntheticPilotItem, 0, mSyntheticPilot);
   mSyntheticPilot->setChecked(false);

   QTreeWidgetItem* guidancePilot = new QTreeWidgetItem(pilotMode, QStringList("    Guidance Pilot"));
   mGuidancePilot                 = new QRadioButton(this);
   vectorTree->setItemWidget(guidancePilot, 0, mGuidancePilot);
   mGuidancePilot->setChecked(false);

   pilotMode->setExpanded(true);

   QTreeWidgetItem* resItem = new QTreeWidgetItem(vectorTree->invisibleRootItem(), QStringList("Resolution"));
   mResolution              = new QComboBox(this);
   mResolution->addItem("960x540");
   mResolution->addItem("960x1080");
   mResolution->addItem("960x1200");
   mResolution->addItem("1920x1080");
   mResolution->addItem("1920x1200");
   mResolution->addItem("1920x2160");
   mResolution->addItem("3840x2160");
   vectorTree->setItemWidget(resItem, 1, mResolution);
}

void WkP6DOF_Controller::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mShowHud        = (mShowHud->checkState(0) == Qt::Checked);
   aPrefData.mFirstPerson    = (mFirstPerson->checkState(0) == Qt::Checked);
   aPrefData.mHDD            = (mHDD->checkState(0) == Qt::Checked);
   aPrefData.mAutopilot      = mAutopilot->isChecked();
   aPrefData.mManualPilot    = mManualPilot->isChecked();
   aPrefData.mSyntheticPilot = mSyntheticPilot->isChecked();
   aPrefData.mGuidancePilot  = mGuidancePilot->isChecked();

   QString      text = mResolution->currentText();
   unsigned int resX = 960;
   unsigned int resY = 540;
   if (text == "960x540")
   {
      resX = 960;
      resY = 540;
   }
   else if (text == "960x1080")
   {
      resX = 960;
      resY = 1080;
   }
   else if (text == "960x1200")
   {
      resX = 960;
      resY = 1200;
   }
   else if (text == "1920x1080")
   {
      resX = 1920;
      resY = 1080;
   }
   else if (text == "1920x1200")
   {
      resX = 1920;
      resY = 1200;
   }
   else if (text == "1920x2160")
   {
      resX = 1920;
      resY = 2160;
   }
   else if (text == "3840x2160")
   {
      resX = 3840;
      resY = 2160;
   }
   aPrefData.mResX = resX;
   aPrefData.mResY = resY;
}

void WkP6DOF_Controller::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mShowHud->setCheckState(0, aPrefData.mShowHud ? Qt::Checked : Qt::Unchecked);
   mFirstPerson->setCheckState(0, aPrefData.mFirstPerson ? Qt::Checked : Qt::Unchecked);
   mHDD->setCheckState(0, aPrefData.mHDD ? Qt::Checked : Qt::Unchecked);
   mAutopilot->setChecked(aPrefData.mAutopilot);
   mManualPilot->setChecked(aPrefData.mManualPilot);
   mSyntheticPilot->setChecked(aPrefData.mSyntheticPilot);
   mGuidancePilot->setChecked(aPrefData.mGuidancePilot);

   if ((aPrefData.mResX == 960) && (aPrefData.mResY == 540))
   {
      mResolution->setCurrentText("960x540");
   }
   else if ((aPrefData.mResX == 960) && (aPrefData.mResY == 1080))
   {
      mResolution->setCurrentText("960x1080");
   }
   else if ((aPrefData.mResX == 960) && (aPrefData.mResY == 1200))
   {
      mResolution->setCurrentText("960x1200");
   }
   else if ((aPrefData.mResX == 1920) && (aPrefData.mResY == 1080))
   {
      mResolution->setCurrentText("1920x1080");
   }
   else if ((aPrefData.mResX == 1920) && (aPrefData.mResY == 1200))
   {
      mResolution->setCurrentText("1920x1200");
   }
   else if ((aPrefData.mResX == 1920) && (aPrefData.mResY == 2160))
   {
      mResolution->setCurrentText("1920x2160");
   }
   else if ((aPrefData.mResX == 3840) && (aPrefData.mResY == 2160))
   {
      mResolution->setCurrentText("3840x2160");
   }
   else
   {
      // Default to a 1080 display, the most common display resolution,
      // but this should never happen under normal operation
      mResolution->setCurrentText("1920x1080");
   }
}
