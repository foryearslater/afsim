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

#ifndef TABLEROW_HPP
#define TABLEROW_HPP

#include <QComboBox>
#include <QLineEdit>
#include <QString>

namespace Engage
{
class Dialog;
class TableWidget;

struct TableRowData
{
   bool    mIsLauncherType;
   QString mName;
   QString mWeapon;
   QString mWeaponQuantity;
   QString mTrackerType;
   QString mSide;
   QString mRadarSignature;
   QString mIcon;
};

class TableRow
{
public:
   TableRow(TableWidget* aTablePtr, Dialog* aDialogPtr);
   TableRow(const TableRow&) = delete;
   TableRow& operator=(const TableRow&) = delete;
   ~TableRow();

   TableRowData GetRowData();

private:
   void PopulateWeaponList();
   void PopulateTrackerList();
   void PopulateRadarSignatureList();

   bool         mIsLauncherType;
   TableWidget* mTablePtr;
   QComboBox*   mTypeColumnPtr;
   QComboBox*   mWeaponColumnPtr;
   QLineEdit*   mWeaponQuantityColumnPtr;
   QComboBox*   mTrackerColumnPtr;
   QComboBox*   mRadarSigColumnPtr;
   QComboBox*   mSideColumnPtr;
   QComboBox*   mIconColumnPtr;
};
} // namespace Engage

#endif
