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
#include "ChatPlugin.hpp"

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QTime>

#include "ChatDockWidget.hpp"
#include "ChatPrefObject.hpp"
#include "ChatPrefWidget.hpp"
#include "UtQtSettingsScopes.hpp"
#include "VaCamera.hpp"
#include "VaCameraMotion.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfTrack.hpp"
#include "WkfUtils.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(Chat::Plugin,
                          "Chat",
                          "The Chat plugin creates the Chat dialog which can be used to communicate over the network "
                          "to other applications.  The Chat dialog can be accessed through the View menu.",
                          "warlock")

Chat::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new PrefWidget())
{
   connect(&mNetwork, &Network::ReceivedMessage, this, &Plugin::OnReceiveMessage);
   connect(&mNetwork, &Network::ReceivedJoinChannel, this, &Plugin::OnReceiveJoinChannel);
   connect(&mNetwork, &Network::ReceivedLeaveChannel, this, &Plugin::OnReceiveLeaveChannel);
   connect(&mNetwork, &Network::ReceivedRename, this, &Plugin::OnReceiveRename);
   connect(&mNetwork, &Network::ConnectionError, this, &Plugin::OnConnectionError);

   auto* dock = CreateDockWidget(Qt::BottomDockWidgetArea);
   dock->setVisible(false);
   CreateGroup(dock, "new");

   auto* prefObj = mPrefWidgetPtr->GetPreferenceObject();
   connect(prefObj, &PrefObject::UserNameChanged, this, &Plugin::OnUserRename);
   connect(prefObj, &PrefObject::TimeStampsChanged, this, &Plugin::OnTimeStampsChanged);
}

QList<wkf::PrefWidget*> Chat::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void Chat::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      BuildPlatformContextMenu(aMenu, QString::fromStdString(aEntityPtr->GetName()));
   }
   else if (aEntityPtr->IsA_TypeOf<wkf::Track>())
   {
      BuildTrackContextMenu(aMenu, QString::fromStdString(aEntityPtr->GetName()));
   }
}

void Chat::Plugin::BuildPlatformContextMenu(QMenu* aMenu, const QString& aName)
{
   auto connectAction = new QAction(QString("Copy Chat Link to Clipboard"), aMenu);
   aMenu->addAction(connectAction);
   connectAction->setData(aName);
   connect(connectAction, &QAction::triggered, this, &Chat::Plugin::CopyChatPlatformLink);
}

void Chat::Plugin::BuildTrackContextMenu(QMenu* aMenu, const QString& aName)
{
   auto connectAction = new QAction(QString("Copy Chat Link to Clipboard"), aMenu);
   aMenu->addAction(connectAction);
   connectAction->setData(aName);
   connect(connectAction, &QAction::triggered, this, &Chat::Plugin::CopyChatTrackLink);
}

void Chat::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   if (aViewerPtr == vaEnv.GetStandardViewer())
   {
      auto connectAction = new QAction(QString("Copy Chat Link to Current View to Clipboard"), aMenu);
      aMenu->addAction(connectAction);
      connect(connectAction, &QAction::triggered, this, &Chat::Plugin::CopyChatViewLink);
   }
}

void Chat::Plugin::CopyChatPlatformLink()
{
   QAction* sender = dynamic_cast<QAction*>(QObject::sender());

   QString platformName = sender->data().toString();
   QString url          = "{p:" + platformName + "}";
   QApplication::clipboard()->setText(url);
}

void Chat::Plugin::CopyChatTrackLink()
{
   QAction* sender = dynamic_cast<QAction*>(QObject::sender());

   QString platformName = sender->data().toString();
   QString url          = "{t:" + platformName + "}";
   QApplication::clipboard()->setText(url);
}

void Chat::Plugin::CopyChatViewLink()
{
   auto* viewerPtr = vaEnv.GetStandardViewer();
   if (viewerPtr)
   {
      QApplication::clipboard()->setText(viewerPtr->GetViewLink());
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

const QString& Chat::Plugin::GetUserName() const
{
   return mUserName;
}

// static
bool Chat::Plugin::IsChannelReserved(const QString& aName)
{
   if (aName == "help")
   {
      return true;
   }
   else if (aName == "join")
   {
      return true;
   }
   else if (aName == "leave")
   {
      return true;
   }
   else if (aName == "channels")
   {
      return true;
   }
   else if (aName == "who")
   {
      return true;
   }
   return false;
}

bool Chat::Plugin::IsGroupNameAvailable(const QString& aNameCandidate, const Group* aGroup /*= nullptr*/) const
{
   for (const auto& group : mGroups)
   {
      if (group && group != aGroup && group->objectName() == aNameCandidate)
      {
         return false;
      }
   }
   return true;
}

QString Chat::Plugin::GetAvailableGroupName(const QString& aNameCandidate, const Group* aGroup /*= nullptr*/) const
{
   if (IsGroupNameAvailable(aNameCandidate, aGroup))
   {
      return aNameCandidate;
   }
   int suffix = 1;
   while (true)
   {
      QString name = aNameCandidate + "_" + QString::number(suffix);
      if (IsGroupNameAvailable(name, aGroup))
      {
         return name;
      }
      suffix++;
   }
}

void Chat::Plugin::SaveSettings(QSettings& aSettings)
{
   ut::qt::WriteArrayScope was{aSettings, "widgets"};
   for (const DockWidget* widget : mDockWidgets)
   {
      if (widget)
      {
         was.Next();
         aSettings.setValue("visible", widget->isVisible());
         aSettings.setValue("geometry", widget->saveGeometry());
         ut::qt::WriteArrayScope was2{aSettings, "groups"};
         const Group*            grp = widget->GetGroup(0);
         int                     i   = 0;
         while (grp)
         {
            was2.Next();
            aSettings.setValue("name", grp->objectName());
            aSettings.setValue("channels", grp->GetChannelList());

            i++;
            grp = widget->GetGroup(i);
         }
      }
   }
}

void Chat::Plugin::LoadSettings(QSettings& aSettings)
{
   ut::qt::ReadArrayScope ras{aSettings, "widgets"};
   if (ras.Size() > 0)
   {
      // Reset
      for (auto* widget : mDockWidgets)
      {
         widget->deleteLater();
      }
      for (auto* group : mGroups)
      {
         group->deleteLater();
      }
      mDockWidgets.clear();
      mGroups.clear();

      // Read
      while (ras.Next())
      {
         DockWidget* dock = CreateDockWidget(Qt::BottomDockWidgetArea);
         dock->setVisible(aSettings.value("visible", true).toBool());
         dock->restoreGeometry(aSettings.value("geometry").toByteArray());
         ut::qt::ReadArrayScope ras2{aSettings, "groups"};
         while (ras2.Next())
         {
            Group*      grp      = CreateGroup(dock, aSettings.value("name", "new").toString());
            QStringList channels = aSettings.value("channels").toStringList();
            for (const QString& channel : channels)
            {
               grp->JoinChannel(channel);
            }
         }
      }
   }
}

void Chat::Plugin::OnUserInput(Group* aGroup, const QString& aInput)
{
   if (aInput.startsWith('/'))
   {
      const int space = aInput.indexOf(' ');
      QString   args;
      if (space > 0)
      {
         args = aInput.mid(space).trimmed();
      }
      OnUserCommand(aGroup, aInput.mid(1, space - 1), args);
   }
   else
   {
      OnUserMessage(aGroup, aGroup->GetActiveChannel(), aInput.trimmed());
   }
}

void Chat::Plugin::OnUserCommand(Group* aGroup, const QString& aCommand, const QString& aArgs)
{
   if (aCommand == "join")
   {
      OnUserJoinChannel(aGroup, aArgs);
   }
   else if (aCommand == "leave")
   {
      OnUserLeaveChannel(aGroup, aArgs);
   }
   else if (aCommand == "help")
   {
      aGroup->WriteHelp();
   }
   else if (aCommand == "who")
   {
      aGroup->WriteUserList(mUserList.value(aGroup->GetActiveChannel()));
   }
   else if (aCommand == "channels")
   {
      aGroup->WriteChannelList();
   }
   else if (aGroup->IsListeningToChannel(aCommand))
   {
      aGroup->SwitchChannel(aCommand);
      // Sending message to channel called <aCommand>
      OnUserMessage(aGroup, aCommand, aArgs);
   }
   else
   {
      OnUserUnrecognizedCommand(aGroup, aCommand);
   }
}

void Chat::Plugin::OnUserMessage(Group* aGroup, const QString& aChannel, const QString& aMessage)
{
   if (aChannel.isEmpty())
   {
      aGroup->WriteColorLine("FF6666", "This tab is not currently writing to any channels.");
   }
   else if (!aMessage.isEmpty())
   {
      // Send to network.
      if (!mNetwork.WriteMessage(GetUserName(), aChannel, aMessage))
      {
         aGroup->WriteColorLine("FF6666", "Network error: message not sent.");
      }

      // Send to open channels.
      const QString message = wkf::util::ParseLinks(aMessage.toHtmlEscaped());
      const QString form    = "%1%2%3 %4";
      WriteToChannel(aChannel, form.arg(GetTimeStamp(), GetChannelLabel(aChannel), GetSelfNameTag(), message));
      mInterfacePtr->MessageSent(GetUserName(), aChannel, aMessage);
   }
}

void Chat::Plugin::OnUserJoinChannel(Group* aGroup, const QString& aChannel)
{
   if (IsChannelReserved(aChannel))
   {
      aGroup->WriteColorLine("FF6666", aChannel + " is a reserved channel name.");
   }
   else if (aChannel.contains(' '))
   {
      aGroup->WriteColorLine("FF6666", "Channel names may not contain spaces.");
   }
   else if (aGroup->JoinChannel(aChannel))
   {
      const QString form = "%1%2 has joined channel %3.";
      WriteToChannel(aChannel, form.arg(GetTimeStamp(), GetSelfNameTag(), GetChannelLabel(aChannel)));
      if (!mNetwork.WriteJoinChannel(GetUserName(), aChannel))
      {
         aGroup->WriteColorLine("FF6666", "Network error: join notification not sent.");
      }
      // Insert aChannel into the user list.
      mUserList.insert(aChannel, {GetUserName()});
   }
}

void Chat::Plugin::OnUserLeaveChannel(Group* aGroup, const QString& aChannel)
{
   if (aGroup->LeaveChannel(aChannel))
   {
      const QString form = "%1%2 has left channel %3.";
      const QString msg  = form.arg(GetTimeStamp(), GetSelfNameTag(), GetChannelLabel(aChannel));
      WriteToChannel(aChannel, msg);
      aGroup->WriteHtml(msg); // The group must be written to directly since it already left the channel.
      if (!mNetwork.WriteLeaveChannel(GetUserName(), aChannel))
      {
         aGroup->WriteColorLine("FF6666", "Network error: leave notification not sent.");
      }
      mUserList.remove(aChannel);
   }
}

void Chat::Plugin::OnUserRename(const QString& aNewName)
{
   if (mUserName != aNewName)
   {
      QString oldName = mUserName;
      mUserName       = aNewName;
      if (!oldName.isEmpty())
      {
         const QString& form = "%1%2 has changed names to %3.";
         WriteToSystem(form.arg(GetTimeStamp(), GetNameTag(oldName), GetSelfNameTag()));
         if (!mNetwork.WriteRename(oldName, aNewName))
         {
            WriteToSystem("<font color=#FF6666>Network error: rename notification not sent.</font>");
         }
         RenameUser(oldName, aNewName);
      }
   }
}

void Chat::Plugin::OnUserUnrecognizedCommand(Group* aGroup, const QString& aCommand)
{
   aGroup->WriteColorLine("FF6666", "Unrecognized command or channel: " + aCommand);
}

void Chat::Plugin::OnReceiveMessage(const QString& aUser, const QString& aChannel, const QString& aMessage)
{
   const QString message = wkf::util::ParseLinks(aMessage.toHtmlEscaped());
   const QString form    = "%1%2%3 %4";
   WriteToChannel(aChannel, form.arg(GetTimeStamp(), GetChannelLabel(aChannel), GetNameTag(aUser), message));
   mInterfacePtr->MessageReceived(aUser, aChannel, aMessage);
   AddUser(aChannel, aUser);
}

void Chat::Plugin::OnReceiveJoinChannel(const QString& aUser, const QString& aChannel)
{
   const QString form = "%1%2 has joined channel %3.";
   WriteToChannel(aChannel, form.arg(GetTimeStamp(), GetNameTag(aUser), GetChannelLabel(aChannel)));
   AddUser(aChannel, aUser);
}

void Chat::Plugin::OnReceiveLeaveChannel(const QString& aUser, const QString& aChannel)
{
   const QString form = "%1%2 has left channel %3.";
   WriteToChannel(aChannel, form.arg(GetTimeStamp(), GetNameTag(aUser), GetChannelLabel(aChannel)));
   RemoveUser(aChannel, aUser);
}

void Chat::Plugin::OnReceiveRename(const QString& aUser, const QString& aNewName)
{
   const QString form = "%1%2 has changed names to %3.";
   WriteToSystem(form.arg(GetTimeStamp(), GetNameTag(aUser), GetNameTag(aNewName)));
   RenameUser(aUser, aNewName);
}

void Chat::Plugin::OnConnectionError(const QString& aInterface, int aPort, const QString& aError)
{
   const QString form =
      "<font color=#FF6666><b>Could not connect to network %1:%2 (%3). Adjust your preferences.</b></font>";
   WriteToSystem(form.arg(aInterface).arg(aPort).arg(aError));
}

void Chat::Plugin::OnGroupCreateRequested(DockWidget* aParent)
{
   CreateGroup(aParent, "new");
}

void Chat::Plugin::OnGroupDestroyRequested(DockWidget* aParent, Group* aGroup)
{
   mGroups.removeAll(aGroup);
   if (aParent->TakeGroup(aGroup))
   {
      aGroup->deleteLater();
   }
}

void Chat::Plugin::OnGroupUndockRequested(DockWidget* aParent, Group* aGroup)
{
   if (aParent->TakeGroup(aGroup))
   {
      auto* dock = CreateDockWidget(Qt::NoDockWidgetArea);
      dock->InsertGroup(aGroup);
   }
}

void Chat::Plugin::OnGroupRenameRequested(DockWidget* aParent, Group* aGroup, const QString& aNewName)
{
   if (aGroup->objectName() != aNewName)
   {
      QString name = GetAvailableGroupName(aNewName, aGroup);
      aGroup->setObjectName(name);
      aParent->SetGroupName(aGroup, name);
   }
}

void Chat::Plugin::OnDockWidgetDestroyed(QObject* aObject)
{
   mDockWidgets.removeAll(static_cast<DockWidget*>(aObject));
}

void Chat::Plugin::OnTimeStampsChanged(bool aWall, bool aSim)
{
   mWallTimeStamp = aWall;
   mSimTimeStamp  = aSim;
}

QString Chat::Plugin::GetTimeStamp() const
{
   if (mWallTimeStamp)
   {
      if (mSimTimeStamp)
      {
         const QString form = "(%1 (%2))";
         return form.arg(QTime::currentTime().toString()).arg(mInterfacePtr->GetSimTime());
      }
      else
      {
         const QString form = "(%1)";
         return form.arg(QTime::currentTime().toString());
      }
   }
   else
   {
      if (mSimTimeStamp)
      {
         const QString form = "((%1))";
         return form.arg(mInterfacePtr->GetSimTime());
      }
      else
      {
         return QString{};
      }
   }
}

QString Chat::Plugin::GetSelfNameTag() const
{
   return "[<b><i>" + GetUserName() + "</i></b>]";
}

// static
QString Chat::Plugin::GetNameTag(const QString& aUser)
{
   return "[" + aUser + "]";
}

// static
QString Chat::Plugin::GetChannelLabel(const QString& aChannel)
{
   const QString form = "[<font color=#%1>%2</font>]";
   return form.arg(wkf::util::GetHexColor(aChannel), aChannel);
}

void Chat::Plugin::WriteToSystem(const QString& aHtml)
{
   for (const auto& group : mGroups)
   {
      if (group)
      {
         group->WriteHtml(aHtml);
      }
   }
}

void Chat::Plugin::WriteToChannel(const QString& aChannel, const QString& aHtml)
{
   for (const auto& group : mGroups)
   {
      if (group && group->IsListeningToChannel(aChannel))
      {
         group->WriteHtml(aHtml);
      }
   }
}

void Chat::Plugin::AddUser(const QString& aChannel, const QString& aName)
{
   auto it = mUserList.find(aChannel);
   if (it != mUserList.end())
   {
      if (!it->contains(aName))
      {
         it->append(aName);
      }
      mNetwork.WriteRollCall(mUserName, aChannel, aName);
   }
}

void Chat::Plugin::RemoveUser(const QString& aChannel, const QString& aName)
{
   auto it = mUserList.find(aChannel);
   if (it != mUserList.end())
   {
      it->removeAll(aName);
   }
}

void Chat::Plugin::RenameUser(const QString& aOldName, const QString& aNewName)
{
   for (auto& channel : mUserList)
   {
      const int index = channel.indexOf(aOldName);
      if (index >= 0)
      {
         channel.replace(index, aNewName);
      }
   }
}

Chat::DockWidget* Chat::Plugin::CreateDockWidget(Qt::DockWidgetArea aArea)
{
   auto* retval = new DockWidget(wkfEnv.GetMainWindow(), mDockWidgets.size());
   mDockWidgets.append(retval);

   connect(retval, &DockWidget::UserInput, this, &Plugin::OnUserInput);
   connect(retval, &DockWidget::AddGroupClicked, this, &Plugin::OnGroupCreateRequested);
   connect(retval, &DockWidget::DeleteGroupClicked, this, &Plugin::OnGroupDestroyRequested);
   connect(retval, &DockWidget::UndockGroupClicked, this, &Plugin::OnGroupUndockRequested);
   connect(retval, &DockWidget::UserRenamedGroup, this, &Plugin::OnGroupRenameRequested);
   connect(retval, &DockWidget::destroyed, this, &Plugin::OnDockWidgetDestroyed);

   wkfEnv.GetMainWindow()->addDockWidget(aArea, retval);
   if (aArea == Qt::NoDockWidgetArea)
   {
      retval->setFloating(true);
   }
   retval->show();

   return retval;
}

Chat::Group* Chat::Plugin::CreateGroup(DockWidget* aParent, const QString& aNameCandidate)
{
   auto* retval = new Group(nullptr, GetAvailableGroupName(aNameCandidate));
   mGroups.append(retval);
   aParent->InsertGroup(retval);

   return retval;
}

void Chat::Plugin::SetGroupName(Group* aGroup, const QString& aNameCandidate)
{
   aGroup->setObjectName(GetAvailableGroupName(aNameCandidate));
}
