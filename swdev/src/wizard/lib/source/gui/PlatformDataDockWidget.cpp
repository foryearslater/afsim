// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "PlatformDataDockWidget.hpp"

#include "Platform.hpp"
#include "PlatformDataTreeWidgetItem.hpp"
#include "WkfEnvironment.hpp"

wizard::PlatformDataDockWidget::PlatformDataDockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : wkf::PlatformDataDockWidget(parent, f)
{
}

void wizard::PlatformDataDockWidget::ItemChanged(const wkf::ChangeInfo& aInfo)
{
   // if this is a change to anything in column '0' that means the user is trying to change something they shouldn't,
   // change it back.... This isn't the ideal way to solve this problem, but that ideal solution requires drastic
   // changes spanning multiple applications for a small problem see the QAbstractItemModel:
   // https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
   if (aInfo.column == 0)
   {
      QString dataName = aInfo.changedPtr->text(0).toLower();
      // find the location of this item in the treewidget
      QTreeWidgetItem* parentWidget = aInfo.changedPtr->parent();
      if (parentWidget != nullptr)
      {
         int idx = parentWidget->indexOfChild(aInfo.changedPtr);
         if (parentWidget->text(0) == "Info")
         {
            if (idx == 0 && dataName != "side")
            {
               aInfo.changedPtr->setText(0, "Side");
            }
            else if (idx == 1 && dataName != "type")
            {
               aInfo.changedPtr->setText(0, "Type");
            }
            else if (idx == 2 && dataName != "icon")
            {
               aInfo.changedPtr->setText(0, "Icon");
            }
         }
         else if (parentWidget->text(0) == "Initial State")
         {
            if (idx == 0 && dataName != "latitude")
            {
               aInfo.changedPtr->setText(0, "Latitude");
            }
            else if (idx == 1 && dataName != "longitude")
            {
               aInfo.changedPtr->setText(0, "Longitude");
            }
            else if (idx == 2 && dataName != "altitude agl")
            {
               aInfo.changedPtr->setText(0, "Altitude AGL");
            }
            else if (idx == 3 && dataName != "altitude msl")
            {
               aInfo.changedPtr->setText(0, "Altitude MSL");
            }
            else if (idx == 4 && dataName != "heading")
            {
               aInfo.changedPtr->setText(0, "Heading");
            }
            else if (idx == 5 && dataName != "pitch")
            {
               aInfo.changedPtr->setText(0, "Pitch");
            }
            else if (idx == 6 && dataName != "roll")
            {
               aInfo.changedPtr->setText(0, "Roll");
            }
         }
      }
   }
   else
   {
      wkf::PlatformList platformList = wkfEnv.GetSelectedPlatforms();
      for (auto platformPtr : platformList)
      {
         // only allow changes from the platform details plugin to effect it's current platform
         if (platformPtr->GetName() == mPlatformName)
         {
            QString dataName = aInfo.changedPtr->text(0).toLower();
            QString dataValue;
            if (aInfo.text == "")
            {
               dataValue = aInfo.changedPtr->text(aInfo.column);
            }
            else
            {
               dataValue = aInfo.text;
            }
            wizard::Platform* wizardPlatformPtr = dynamic_cast<wizard::Platform*>(platformPtr);

            auto altitudeWidgetPtr = dynamic_cast<wizard::ValidatedAltitudeTreeWidgetItem*>(aInfo.changedPtr);
            auto angleWidgetPtr    = dynamic_cast<wizard::ValidatedAngleTreeWidgetItem*>(aInfo.changedPtr);

            // If this change was not just a unit adjustment, update the text editor platform
            if (!(altitudeWidgetPtr && altitudeWidgetPtr->GetUnitChanged()) &&
                !(angleWidgetPtr && angleWidgetPtr->GetUnitChanged()) && wizardPlatformPtr != nullptr)
            {
               wizardPlatformPtr->CheckPlatformDataChange(dataName, dataValue);
            }
         }
      }
   }
}
