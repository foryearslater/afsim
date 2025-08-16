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
#ifndef WkScriptBrowserDockWidget_HPP
#define WkScriptBrowserDockWidget_HPP

#include <QDockWidget>
#include <QSettings>
#include <QTreeWidgetItem>
#include <QWidget>

#include "VaCallbackHolder.hpp"
#include "WkScriptSimInterface.hpp"
#include "WkfPlatform.hpp"
#include "WkfTrack.hpp"
#include "WsfGeoPoint.hpp"
#include "ui_ScriptBrowserDockWidget.h"

namespace WkScriptBrowser
{
class DataWidget : public QWidget
{
public:
   DataWidget(const QString& aType, QWidget* parent = nullptr);
   ~DataWidget() override;

   warlock::script::Argument GetScriptArg();

   bool              GetBool() const;
   int               GetInt() const;
   QString           GetString() const;
   double            GetDouble() const;
   QVector<double>   GetLocation() const;
   QString           GetPlatform() const;
   wkf::TrackId      GetTrackId() const;
   QVector<QVariant> GetArray() const;

   void CancelSelection();

private:
   warlock::script::DataType mType;
   QWidget*                  mWidget;
   QString                   mTypeStr;
};

class DockWidget : public QDockWidget
{
   Q_OBJECT
public:
   DockWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~DockWidget() override = default;

   void SimulationStarting();

private:
   void RemotePlatformUpdatedCB(const QString& aPlatformName);
   void ReturnValueReadyCB(int aScriptId, const warlock::script::ReturnValue& aReturnValue);

   void ExecutePushButtonClicked();
   void TreeWidgetItemSelectionChanged();

   enum roles
   {
      PLATFORM_NAME = Qt::UserRole,
      SCRIPT_NAME   = Qt::UserRole + 1
   };

   void PlatformSelectionChanged(const wkf::PlatformList& aSelectedPlatforms,
                                 const wkf::PlatformList& aUnselectedPlatforms);

   void RemovePlatformTree(const QString& aPlatformName);
   void ClearScriptInfo();
   void ClearReturnValue();
   void SetShowReturnValue(bool aShow);

   void PopulateGlobalTableWidget();
   void PopulatePlatformTableWidget(const QString& aPlatformName);
   void PopulateScrollArea(const warlock::script::Data& aData, const QString aPlatformName = "");

   bool    IsWarlockScript(const QString& aScriptName) const;
   QString RemoveWarlockPrefix(const QString& aScriptName) const;

   vespa::VaCallbackHolder mCallbacks;

   std::shared_ptr<warlock::ScriptSimInterface> mSimInterfacePtr;

   const QString mScriptPrefix;

   std::set<int> mScriptsWaitingForReturnValues;

   QTreeWidgetItem*                    mGlobalTopLevelItem;
   std::map<QString, QTreeWidgetItem*> mPlatformTopLevelItemMap;
   // Widget containing the dynamically created widgets containing the script arguments
   QWidget* mArgWidget;
   // List containing the widgets representing the arguments for the currently selected script
   // This list is a subset of widget contained in mScrollAreaWidgets
   QList<DataWidget*> mScriptArgWidgets;

   Ui::ScriptBrowserDockWidget mUi;
};
} // namespace WkScriptBrowser

#endif
