// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TASKSTATUSDOCKWIDGET_HPP
#define TASKSTATUSDOCKWIDGET_HPP

#include <QDockWidget>

#include "WkfPlatform.hpp"
#include "WsfTaskManager.hpp"
#include "ui_TaskStatusDockWidget.h"

class WsfTask;

namespace WkTaskStatus
{
class DataContainer;
class SimInterface;

class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   DockWidget(SimInterface*   aSimInterfacePtr,
              DataContainer&  aData,
              QWidget*        aParentPtr = nullptr,
              Qt::WindowFlags aFlags     = Qt::WindowFlags());
   ~DockWidget() override = default;

protected:
   enum ENTRY_TYPE
   {
      cASSIGNED = 1,
      cRECEIVED = 2,
      cTRACK    = 3,
      cTASK     = 4
   };

   enum Role
   {
      cPLATFORM_ROLE   = Qt::UserRole + 1,
      cPROCESSOR_ROLE  = Qt::UserRole + 2,
      cTASKID_ROLE     = Qt::UserRole + 3,
      cTRACKINDEX_ROLE = Qt::UserRole + 4,
   };

   const char* cPLATFORM_STR  = "p";
   const char* cPROCESSOR_STR = "r";
   const char* cTASKID_STR    = "i";
   const char* cTRACK_STR     = "t";

   void TreeWidgetContextMenu(const QPoint& aPoint);

private:
   void Connect();

   void AddTasks(const std::string& aTaskManagerName, const std::map<WsfTrackId, WsfTaskManager::TaskList>& aTaskList);
   void CancelTaskHandler();
   void CancelAllTasksHandler();
   void AbortFiringHandler();

   QString GetTaskResourceType(const WsfTask& aTask);

   void PlatformOfInterestChanged(wkf::Platform* aPlatformPtr);
   void TaskChanged();

   DataContainer& mData;
   SimInterface*  mSimInterfacePtr;

   Ui::TaskStatusDockWidget mUi;
};
} // namespace WkTaskStatus

#endif // TASKSTATUSDOCKWIDGET_HPP
