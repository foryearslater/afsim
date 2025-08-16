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
#ifndef CHATPLUGIN_HPP
#define CHATPLUGIN_HPP

#include "ChatDockWidget.hpp"
#include "ChatNetwork.hpp"
#include "ChatPrefWidget.hpp"
#include "ChatSimInterface.hpp"
#include "WkPlugin.hpp"

namespace Chat
{
//! Almost everything "interesting" that happens in Chat is funneled through the Plugin class's callbacks.
//! The callbacks process and filter input (e.g. commands, messages, network traffic)
//!    and notify interested components (e.g. network, preferences, sim interface, widgets).
class Plugin : public warlock::PluginT<Chat::SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;
   void                    BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;
   void                    BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;
   void                    CopyChatPlatformLink();
   void                    CopyChatTrackLink();
   void                    CopyChatViewLink();

   const QString& GetUserName() const;
   static bool    IsChannelReserved(const QString& aChannel);

   //! If aGroup is non-null, will return true if aGroup has name aNameCandidate.
   bool IsGroupNameAvailable(const QString& aNameCandidate, const Group* aGroup = nullptr) const;
   //! If aGroup is non-null, will not consider its current name taken.
   QString GetAvailableGroupName(const QString& aNameCandidate, const Group* aGroup = nullptr) const;

   void SaveSettings(QSettings& aSettings) override;
   void LoadSettings(QSettings& aSettings) override;

private:
   //! Called when the user inputs a message or command.
   void OnUserInput(Group* aGroup, const QString& aInput);
   //! OnUserInput() delegates to these functions.
   //{
   void OnUserCommand(Group* aGroup, const QString& aCommand, const QString& aArgs);
   void OnUserMessage(Group* aGroup, const QString& aChannel, const QString& aMessage);
   void OnUserJoinChannel(Group* aGroup, const QString& aChannel);
   void OnUserLeaveChannel(Group* aGroup, const QString& aChannel);
   void OnUserRename(const QString& aNewName);
   void OnUserUnrecognizedCommand(Group* aGroup, const QString& aCommand);
   //}

   void OnConnectionError(const QString& aInterface, int aPort, const QString& aError);

   //! Called by mNetwork when packets are received.
   //{
   void OnReceiveMessage(const QString& aUser, const QString& aChannel, const QString& aMessage);
   void OnReceiveJoinChannel(const QString& aUser, const QString& aChannel);
   void OnReceiveLeaveChannel(const QString& aUser, const QString& aChannel);
   void OnReceiveRename(const QString& aUser, const QString& aNewName);
   //}

   //! Called when a Group is edited by a DockWidget.
   //{
   void OnGroupCreateRequested(DockWidget* aParent);
   void OnGroupDestroyRequested(DockWidget* aParent, Group* aGroup);
   void OnGroupUndockRequested(DockWidget* aParent, Group* aGroup);
   void OnGroupRenameRequested(DockWidget* aParent, Group* aGroup, const QString& aNewName);
   //}

   void OnDockWidgetDestroyed(QObject* aObject);
   void OnTimeStampsChanged(bool aWall, bool aSim);

   //! Returns the string to display for the time stamp on messages.
   //! The string can be affected by user preferences.
   QString        GetTimeStamp() const;
   QString        GetSelfNameTag() const;
   static QString GetNameTag(const QString& aUser);
   static QString GetChannelLabel(const QString& aChannel);

   //! Writes a line to every Group.
   void WriteToSystem(const QString& aHtml);
   //! Writes a line to every Group listening to aChannel.
   void WriteToChannel(const QString& aChannel, const QString& aHtml);

   //! Adds a user to mUserList.
   //! If aChannel is not a key in the user list, does nothing.
   //! In aName is already present in the channel, does nothing.
   void AddUser(const QString& aChannel, const QString& aName);
   //! Removes a user from mUserList.
   //! If aChannel is not a key in the user list, does nothing.
   void RemoveUser(const QString& aChannel, const QString& aName);
   //! Replaces all instances of aOldName with aNewName in mUserList.
   void RenameUser(const QString& aOldName, const QString& aNewName);

   void BuildPlatformContextMenu(QMenu* aMenu, const QString& aName);
   void BuildTrackContextMenu(QMenu* aMenu, const QString& aName);

   //! Creates a dock widget and adds it to mDockWidgets.
   DockWidget* CreateDockWidget(Qt::DockWidgetArea aArea);
   //! Creates a group on aParent and adds it to mGroups.
   Group* CreateGroup(DockWidget* aParent, const QString& aNameCandidate);
   //! Renames aGroup with aNameCandidate if name is not already taken.
   //! Otherwise, appends an available number to aNameCandidate.
   void SetGroupName(Group* aGroup, const QString& aNameCandidate);

   QString mUserName;
   bool    mWallTimeStamp = true;
   bool    mSimTimeStamp  = true;

   Network                     mNetwork;
   QMap<QString, QStringList>  mUserList;
   QList<DockWidget*>          mDockWidgets;
   QList<Group*>               mGroups;
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;
};
} // namespace Chat

#endif
