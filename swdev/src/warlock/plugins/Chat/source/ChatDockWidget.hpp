// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef CHATDOCKWIDGET_HPP
#define CHATDOCKWIDGET_HPP

#include <QDockWidget>
class QLabel;
#include <QLineEdit>
#include <QPointer>
class QTabWidget;

#include "ChatGroup.hpp"
#include "ChatNetwork.hpp"
#include "ChatPrefObject.hpp"
#include "ChatTabWidget.hpp"

namespace Chat
{

// this LineEdit emits a ChannelCycle and blocks the focus change on tab key presses
// it also caches ten lines of input that can be accessed with the up/down arrow keys
class CachingLineEdit : public QLineEdit
{
   Q_OBJECT
public:
   CachingLineEdit(QWidget* aParent = nullptr)
      : QLineEdit(aParent)
      , mCacheIdx(-1)
   {
   }
   ~CachingLineEdit() override = default;

signals:
   void ChannelCycle();

private:
   void focusOutEvent(QFocusEvent* aEvent) override;
   void keyPressEvent(QKeyEvent* aEvent) override;

   QStringList mCache;
   int         mCacheIdx;
};

//! DockWidgets contain multiple Groups in a QTabWidget, as well as a line edit for user input.
//! All user commands and input are sent to the UserInput method in the Plugin via signals & slots.
//! The Plugin then forwards any output text directly the the Groups, bypassing the DockWidget.
class DockWidget : public QDockWidget
{
   Q_OBJECT
public:
   DockWidget(QWidget* parent, int aId, Qt::WindowFlags f = Qt::WindowFlags());

   ~DockWidget() override = default;
   void MoveGrouping(TabWidget* aSourcePtr, int aIndex);
   void SetId(int aId);
   int  GetId() const { return mId; }

   //! Returns a pointer to the currently open Group.
   Group* GetActiveGroup() const;
   //! Returns a pointer to the Group at the given index.
   Group* GetGroup(int aIndex) const;

   //! Sets the objectName and tab name for aGroup.
   void SetGroupName(Group* aGroup, const QString& aName);

   //! Adds aGroup to *this.
   //! Called by Plugin.
   void InsertGroup(Group* aGroup);
   //! Removes aGroup from *this.
   //! aGroup is not deleted.
   //! If there are no groups remaining, schedules *this for deletion.
   //! Called by Plugin.
   bool TakeGroup(Group* aGroup);

signals:
   //! Emitted when the user enters a command or message.
   //! aGroup is the group that the input was for.
   void UserInput(Group* aGroup, const QString& aInput);

   //! Called if the "+" tab is clicked.
   void AddGroupClicked(DockWidget* aThis);
   //! Called if "Delete group" is selected from the tab context menu.
   //! The Plugin will manage what happens to the Groups and DockWidgets.
   void DeleteGroupClicked(DockWidget* aMaybeThis, Group* aGroup);
   //! Called if "Undock group" is selected from the tab context menu.
   //! The Plugin will manage what happens to the Groups and DockWidgets.
   void UndockGroupClicked(DockWidget* aThis, Group* aGroup);
   //! The new name will be sent to the plugin to check for uniqueness.
   //! The plugin will call SetGroupName() with the new name.
   void UserRenamedGroup(DockWidget* aThis, Group* aGroup, const QString& aName);

private:
   //! Called when the user presses enter in the line edit.
   void OnReturnPressed();
   //! Does the work of creating and laying out the UI.
   void Build();
   void TabClicked(int aIndex);
   void TabDoubleClicked(int aIndex);
   void TabBarContextMenu(QPoint aPos);

   //! If aGroup is not the active group, adds "(*)" to the end of the tab name.
   //! This lets the user know that there is new text to be read.
   void NotifyGroup(Group* aGroup);
   //! If aGroup is the currently active Group, updates the channel label.
   void GroupChangedChannel(Group* aGroup);

   void HelpClicked();

   QLabel*          mChannelLabel;
   CachingLineEdit* mLineEdit;
   TabWidget*       mGroupings;
   bool             mGroupLocked = false;
   int              mId;
};

} // namespace Chat
#endif
