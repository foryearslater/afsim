// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLOTOPTIONSDIALOG_HPP
#define PLOTOPTIONSDIALOG_HPP

#include <QDialog>

#include "ui_PlotOptionsDialog.h"

class QTreeWidgetItem;

namespace SPLAT
{
class PlotOptionsWidget;

class SensorAnalysisDialog;

class PlotOptionsDialog : public QDialog
{
   Q_OBJECT
public:
   PlotOptionsDialog(QWidget* aParent = nullptr);
   ~PlotOptionsDialog() override = default;

   void ApplyAllChanges() { OkClicked(); }

   void    SetTargetPlatformType(const QString& aTargetPlatformType) { mTargetPlatformType = aTargetPlatformType; }
   QString GetProjectDirectory() { return mProjectDirectory; }

   PlotOptionsWidget* GetWidget(const std::string& aName);

signals:
   void UpdateModes(const QStringList& aModeNames);
   void UpdatePlatformTypes(const QStringList& aPlatformTypes);
   void CheckValidity();
   void SetSensorId(const QString& aName);

protected:
   void AddItem(QTreeWidgetItem* aItem, const QString& aToolTip = "", QTreeWidgetItem* aParentItem = nullptr);
   void AddWidget(const std::string& aName, PlotOptionsWidget* aWidgetPtr);
   void ItemSelected(QTreeWidgetItem* aTreeItem, int aColumn);
   void SetWidget(const std::string& aName); // sets the stacked widget to the widget named aName
   PlotOptionsWidget* GetWidget(const int aIndex);

   std::map<std::string, int>      mWidgetIndexMap; // map from option name to index of widget in mWidgetVec;
   std::vector<PlotOptionsWidget*> mWidgetVec;      // widgets in the stackedWidget
   QString                         mTargetPlatformType;
   QString                         mProjectDirectory;

private:
   void OkClicked();
   void Connect();
   void RestoreDefaults();

   QStringList           mModeNames;
   Ui::PlotOptionsDialog mUi;
};
} // namespace SPLAT

#endif
