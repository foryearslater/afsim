// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TableRow.hpp"

#include <QFile>

#include "Dialog.hpp"
#include "TableWidget.hpp"

Engage::TableRow::TableRow(TableWidget* aTablePtr, Dialog* aDialogPtr)
   : mIsLauncherType(false)
   , mTablePtr(aTablePtr)
{
   int curRow = mTablePtr->rowCount();
   mTablePtr->insertRow(curRow);
   // loop through the headeritems and create fields that correspond to those types
   for (int i = 0; i < mTablePtr->columnCount(); i++)
   {
      QString curHeader = mTablePtr->horizontalHeaderItem(i)->text();
      if (curHeader == "Type")
      {
         mTypeColumnPtr = new QComboBox(mTablePtr);
         std::vector<const QString*> typeVec;
         // if this is type create a combo box with all the types
         if (mTablePtr->objectName() == "mLauncherTableWidget")
         {
            typeVec         = aDialogPtr->GetLauncherList();
            mIsLauncherType = true;
         }
         else
         {
            typeVec = aDialogPtr->GetTargetList();
         }
         for (auto&& typeIter : typeVec)
         {
            const QString* curStr = typeIter;
            mTypeColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mTypeColumnPtr);
      }
      else if (curHeader == "Weapon")
      {
         mWeaponColumnPtr                      = new QComboBox(mTablePtr);
         std::vector<const QString*> weaponVec = aDialogPtr->GetWeaponList();
         for (auto&& weaponIter : weaponVec)
         {
            const QString* curStr = weaponIter;
            mWeaponColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mWeaponColumnPtr);
      }
      else if (curHeader == "Weapon Amount")
      {
         mWeaponQuantityColumnPtr = new QLineEdit(mTablePtr);
         mWeaponQuantityColumnPtr->setText("0");
         mTablePtr->setCellWidget(curRow, i, mWeaponQuantityColumnPtr);
      }
      else if (curHeader == "Tracker")
      {
         mTrackerColumnPtr                      = new QComboBox(mTablePtr);
         std::vector<const QString*> trackerVec = aDialogPtr->GetTrackerList();
         for (auto&& trackerIter : trackerVec)
         {
            const QString* curStr = trackerIter;
            mTrackerColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mTrackerColumnPtr);
      }
      else if (curHeader == "Side")
      {
         mSideColumnPtr                      = new QComboBox(mTablePtr);
         std::vector<const QString*> sideVec = aDialogPtr->GetSideList();
         for (auto&& sideIter : sideVec)
         {
            const QString* curStr = sideIter;
            mSideColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mSideColumnPtr);
      }
      else if (curHeader == "Radar Signature")
      {
         mRadarSigColumnPtr                 = new QComboBox(mTablePtr);
         std::vector<const QString*> sigVec = aDialogPtr->GetRadarSigList();
         for (auto&& sigIter : sigVec)
         {
            const QString* curStr = sigIter;
            mRadarSigColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mRadarSigColumnPtr);
      }
      else if (curHeader == "Icon")
      {
         mIconColumnPtr                      = new QComboBox(mTablePtr);
         std::vector<const QString*> iconVec = aDialogPtr->GetIconList();
         for (auto&& iconIter : iconVec)
         {
            const QString* curStr = iconIter;
            mIconColumnPtr->addItem(*curStr);
         }
         mTablePtr->setCellWidget(curRow, i, mIconColumnPtr);
      }
   }
}

Engage::TableRow::~TableRow()
{
   delete mTypeColumnPtr;
   delete mWeaponColumnPtr;
   delete mWeaponQuantityColumnPtr;
   delete mTrackerColumnPtr;
   delete mRadarSigColumnPtr;
   delete mSideColumnPtr;
   delete mIconColumnPtr;
}

Engage::TableRowData Engage::TableRow::GetRowData()
{
   TableRowData rowData;
   rowData.mIsLauncherType = mIsLauncherType;
   rowData.mName           = mTypeColumnPtr->currentText();
   if (mIsLauncherType)
   {
      rowData.mWeapon         = mWeaponColumnPtr->currentText();
      rowData.mWeaponQuantity = mWeaponQuantityColumnPtr->text();
      rowData.mTrackerType    = mTrackerColumnPtr->currentText();
      rowData.mRadarSignature = "";
   }
   else
   {
      rowData.mRadarSignature = mRadarSigColumnPtr->currentText();
      rowData.mIcon           = mIconColumnPtr->currentText();
      rowData.mWeapon         = "";
      rowData.mWeaponQuantity = "";
      rowData.mTrackerType    = "";
   }
   rowData.mSide = mSideColumnPtr->currentText();

   return rowData;
}

void Engage::TableRow::PopulateWeaponList()
{
   Dialog* mwPtr = mTablePtr->GetDialog();
   if (mwPtr)
   {
      QString dirLoc = mwPtr->GetWeaponPath();
      QFile   weaponFile(dirLoc);
      if (!weaponFile.open(QIODevice::ReadOnly | QIODevice::Text))
      {
         return;
      }

      while (!weaponFile.atEnd())
      {
         QString curLine(weaponFile.readLine());
         if (curLine.startsWith("weapon "))
         {
            QStringList lineList = curLine.split(" ");

            mWeaponColumnPtr->addItem(lineList[1]);
         }
      }
   }
}

void Engage::TableRow::PopulateTrackerList()
{
   Dialog* mwPtr = mTablePtr->GetDialog();
   if (mwPtr)
   {
      QString dirLoc = mwPtr->GetTrackerPath();
      QFile   trackerFile(dirLoc);
      if (!trackerFile.open(QIODevice::ReadOnly | QIODevice::Text))
      {
         return;
      }

      while (!trackerFile.atEnd())
      {
         QString curLine(trackerFile.readLine());
         if (curLine.startsWith("platform_type"))
         {
            QStringList lineList = curLine.split(" ");
            mTrackerColumnPtr->addItem(lineList[1]);
         }
      }
   }
}

void Engage::TableRow::PopulateRadarSignatureList()
{
   Dialog* mwPtr = mTablePtr->GetDialog();
   if (mwPtr)
   {
      QString dirLoc = mwPtr->GetTargetPath();
      QFile   radarFile(dirLoc);
      if (!radarFile.open(QIODevice::ReadOnly | QIODevice::Text))
      {
         return;
      }

      while (!radarFile.atEnd())
      {
         QString curLine(radarFile.readLine());
         if (curLine.startsWith("radar_signature"))
         {
            QStringList lineList = curLine.split(" ");
            mRadarSigColumnPtr->addItem(lineList[1].simplified());
         }
      }
   }
}
