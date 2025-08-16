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

#ifndef TASKASSIGNERDOCKWIDGET_HPP
#define TASKASSIGNERDOCKWIDGET_HPP

#include <QDockWidget>
#include <QLabel>
#include <QTreeWidget>

#include "WkfPlatform.hpp"
#include "ui_TaskAssignerDockWidget.h"

namespace wkf
{
class Track;
}

namespace WkTaskAssigner
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

   void Update();

protected:
   void AssigneeSelected(QTreeWidgetItem* aTreeItemPtr, int aColumn);

private:
   void Connect();
   void Disable();
   void Enable(bool aSelf);
   void Reset();

   bool IsAssigneeValid();
   bool IsAssignerValid();
   bool IsTaskValid();
   bool IsTrackValid();

   void AssignButtonPressed();
   void CheckAssignmentConditions();
   void CommandChainComboBoxChanged(int aIndex);
   void FilterChanged();
   bool ItemPassesVisibilityFilter(const QTreeWidgetItem* aItem);
   void PlatformDeleted(const std::string& aPlatformName);
   void PlatformOfInterestChanged(wkf::Platform* aPlatformPtr);
   void SensorComboBoxChanged(int aIndex);
   void SetChecked(QLabel* aLabel, bool aChecked = true);
   void TaskToolBoxChanged(int aIndex);
   void TrackSelected(wkf::Track* aTrack);
   void TrackTextEdited(const QString& aText);
   void WeaponComboBoxChanged(int aIndex);


   DataContainer& mData;
   bool           mPendingUpdate;
   SimInterface*  mSimInterfacePtr;

   Ui::TaskAssignerDockWidget mUi;
};
} // namespace WkTaskAssigner

#endif // TASKMANAGEMENTDOCKWIDGET_HPP
