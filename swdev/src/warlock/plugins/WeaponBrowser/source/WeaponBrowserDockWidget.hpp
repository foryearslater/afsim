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

#ifndef WeaponBrowserTreeDockWidget_HPP
#define WeaponBrowserTreeDockWidget_HPP

#include <QDockWidget>
#include <QLineEdit>

#include "WeaponBrowserDataContainer.hpp"
#include "WeaponBrowserSimInterface.hpp"
#include "WkfSelectorWidgets.hpp"
#include "ui_WeaponBrowserTreeDockWidget.h"

namespace WkWeaponBrowser
{
// custom track command widget for easy insertion into tree structure
class WeaponCommandWidget : public QWidget
{
   Q_OBJECT
public:
   WeaponCommandWidget(QWidget* aParent = nullptr);

   wkf::TrackSelector* mTrackButton;
   QLineEdit*          mTrackEditor;
   QLineEdit*          mQuantityEditor;
   QPushButton*        mFireButton;
   QPushButton*        mDeleteButton;
   QPalette            mDefaultEditStyle;

private:
   QLabel* mTrackLabel;
   QLabel* mQuantityLabel;
};

class DockWidget : public QDockWidget
{
   Q_OBJECT
public:
   DockWidget(SimInterface*   aSimInterface,
              DataContainer&  aDataContainer,
              QWidget*        aParent = nullptr,
              Qt::WindowFlags aFlags  = Qt::WindowFlags());

   ~DockWidget() override = default;
   void Update();
   void LengthUnitChanged(UtUnitLength::BaseUnit aUnit) { Update(); }

private:
   void PlatformOfInterestChanged(wkf::Platform* aPlatform);
   void TrackSelected(WeaponCommandWidget* aRow, wkf::Track* aTrack);
   void AddCommand(QVBoxLayout* aLayout, unsigned int aIndex, const std::string& aName);
   void AddPlusButton(QTreeWidgetItem* aWeaponPtr, const std::string& aName, const std::vector<FiringData>& aData);
   DataContainer& mDataContainer;
   SimInterface*  mSimInterface;

   Ui::WeaponBrowserTreeDockWidget mUi;

   // private connections used to easily disconnect temporary signals associated with a platform of interest
   QMetaObject::Connection mExpandConnection;
   QMetaObject::Connection mCollapseConnection;
};
} // namespace WkWeaponBrowser
#endif
