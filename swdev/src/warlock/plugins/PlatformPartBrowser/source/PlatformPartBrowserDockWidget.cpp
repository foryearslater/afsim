// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPartBrowserDockWidget.hpp"

#include <QCheckBox>
#include <QTreeWidget>
#include <QVariant>

#include "PlatformPartBrowserSimCommands.hpp"
#include "PlatformPartBrowserSimInterface.hpp"
#include "UtMemory.hpp"
#include "WkPermissions.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitTypes.hpp"
#include "WkfVtkEnvironment.hpp"

WkPlatformPartBrowser::DockWidget::DockWidget(SimInterface*   aSimInterface,
                                              DataContainer&  aData,
                                              QWidget*        aParent,
                                              Qt::WindowFlags aFlags)
   : QDockWidget(aParent, aFlags)
   , mSimInterface(aSimInterface)
   , mData(aData)
{
   mUi.setupUi(this);

   mUi.mPlatformPartTreeWidget->setHeaderLabel("");

   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
   connect(&mData, &DataContainer::CheckBoxToggled, this, &DockWidget::HandleCheckBoxChanged);
   connect(&mData, &DataContainer::PlatformPartsSet, this, &DockWidget::Populate);
   connect(mUi.mPlatformPartTreeWidget, &QTreeWidget::itemExpanded, this, &DockWidget::ResizeColumns);
   connect(mUi.mPlatformPartTreeWidget, &QTreeWidget::itemCollapsed, this, &DockWidget::ResizeColumns);
}

void WkPlatformPartBrowser::DockWidget::HandleCheckBoxChanged(const std::string& aPartName,
                                                              PropertyName       aPropertyName,
                                                              bool               aChecked)
{
   auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(mData.GetPlatformOfInterest());
   if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
   {
      QMessageBox::warning(nullptr,
                           "Unable to Modify Platform",
                           "User does not have permissions to modify platform " +
                              QString::fromStdString(platformPtr->GetName()) + ".");
   }
   else
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<ChangePartCommand>(mData.GetPlatformOfInterest(), aPartName, aPropertyName, aChecked));
   }
}

void WkPlatformPartBrowser::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform)
   {
      mSimInterface->SetPlatformName(aPlatform->GetName());
      mData.SetPlatformOfInterest(aPlatform->GetName());
   }
   else
   {
      mSimInterface->SetPlatformName("");
      mData.SetPlatformOfInterest("");
   }
}

// This should only get called when the platform selection changes
void WkPlatformPartBrowser::DockWidget::Populate(const QString& aPlatformName, const PlatformPartList& aPartList)
{
   mUi.mPlatformPartTreeWidget->clear();

   if (aPlatformName.isEmpty())
   {
      mUi.mPlatformPartTreeWidget->setHeaderLabel("");
   }
   else
   {
      mUi.mPlatformPartTreeWidget->setHeaderLabel(aPlatformName);
      mData.CreateTreeWidgetItems(mUi.mPlatformPartTreeWidget);
      ResizeColumns();
   }
}

void WkPlatformPartBrowser::DockWidget::ResizeColumns()
{
   mUi.mPlatformPartTreeWidget->resizeColumnToContents(0);
   mUi.mPlatformPartTreeWidget->resizeColumnToContents(1);
}

void WkPlatformPartBrowser::DockWidget::showEvent(QShowEvent* aEvent)
{
   mSimInterface->SetEnabled(true);
   QWidget::showEvent(aEvent);
}

void WkPlatformPartBrowser::DockWidget::closeEvent(QCloseEvent* aEvent)
{
   mSimInterface->SetEnabled(false);
   QWidget::closeEvent(aEvent);
}
