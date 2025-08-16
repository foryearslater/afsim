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

#include "ChatGroup.hpp"

#include <QContextMenuEvent>
#include <QDateTime>
#include <QMenu>
#include <QMessageBox>

#include "WkfEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfUtils.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

Chat::Group::Group(QWidget* aParent, const QString& aName, Group* aGroup)
   : QPlainTextEdit(aParent)
{
   if (aGroup != nullptr)
   {
      document()->setHtml(aGroup->document()->toHtml());
   }
   setObjectName(aName);
   setReadOnly(true);
   setMouseTracking(true);
   setMaximumBlockCount(1000);
}

void Chat::Group::contextMenuEvent(QContextMenuEvent* e)
{
   QString anchor    = anchorAt(e->pos());
   bool    processed = false;
   if (!anchor.isEmpty())
   {
      QUrl    url(anchor);
      QString scheme = url.scheme();
      QString host   = url.host();
      QString query  = url.query();
      if (scheme == "wkf")
      {
         auto* scenarioPtr = vaEnv.GetStandardScenario();
         if (scenarioPtr)
         {
            if (host == "platform")
            {
               if (!query.isEmpty())
               {
                  wkf::Platform* plat = scenarioPtr->FindPlatformByName(query);
                  if ((plat) && (plat->IsVisible(vaEnv.GetStandardViewer())))
                  {
                     QMenu menu;
                     wkfEnv.BuildEntityContextMenu(&menu, plat);
                     menu.exec(mapToGlobal(e->pos()));
                     processed = true;
                  }
               }
            }
            else if (host == "track")
            {
               if (!query.isEmpty())
               {
                  if (query.indexOf("track_") == 0)
                  {
                     int colonLoc = query.lastIndexOf(':');
                     if (colonLoc != -1)
                     {
                        QString     name  = query.mid(6, colonLoc - 6);
                        int         id    = query.mid(colonLoc + 1).toInt();
                        wkf::Track* track = scenarioPtr->FindTrack(wkf::TrackId(name.toStdString(), id));
                        if ((track) && (track->IsVisible(vaEnv.GetStandardViewer())))
                        {
                           QMenu menu;
                           wkfEnv.BuildEntityContextMenu(&menu, track);
                           menu.exec(mapToGlobal(e->pos()));
                           processed = true;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   if (!processed)
   {
      QMenu menu(this);
      menu.addAction("Save as");
      QAction* result = menu.exec(mapToGlobal(e->pos()));
      if (result)
      {
         // For now, this extra scope is redundant,
         //    but it makes future additions simpler.
         if (result->text() == "Save as")
         {
            SaveAs();
         }
      }
   }
}

QString Chat::Group::GetActiveChannelLabel() const
{
   if (mActiveChannel.isEmpty())
   {
      return "No active channel:";
   }

   const QString form = "[<font color=#%1>%2</font>]";
   return form.arg(wkf::util::GetHexColor(mActiveChannel), mActiveChannel);
}

void Chat::Group::WriteHtml(const QString& aHtml)
{
   QTextCursor prev_cursor = textCursor();
   moveCursor(QTextCursor::End);
   appendHtml(aHtml);
   setTextCursor(prev_cursor);

   emit TextAdded(this);
}

void Chat::Group::WriteColorLine(const QString& aColor, const QString& aHtml)
{
   const QString form = "<font color=#%1>%2</font>";
   WriteHtml(form.arg(aColor, aHtml));
}

void Chat::Group::WriteChannelList()
{
   if (mChannels.empty())
   {
      WriteColorLine("FF6666", "This tab is not on any channels");
   }
   else
   {
      WriteColorLine("AAAAAA", "Channels:");
      for (const auto& it : mChannels)
      {
         if (it == mActiveChannel)
         {
            WriteColorLine("AAAADD", it);
         }
         else
         {
            WriteColorLine("AAAAAA", it);
         }
      }
   }
}

void Chat::Group::WriteUserList(const QStringList& aUsers)
{
   const QString form = "User List(%1) for channel: %2";
   WriteHtml(form.arg(aUsers.size()).arg(GetActiveChannelLabel()));
   for (const QString& user : aUsers)
   {
      WriteHtml(user);
   }
}

void Chat::Group::WriteHelp()
{
   WriteColorLine("AAAAAA", "<b>Help</b>");
   WriteColorLine("AAAAAA", "=========");
   WriteColorLine("AAAAAA", "\'/help\'                - Display this help message");
   WriteColorLine("AAAAAA", "\'/join &lt;channel&gt;\'      - Join the chat group &lt;channel&gt; in this tab");
   WriteColorLine("AAAAAA", "\'/leave &lt;channel&gt;\'     - Leave the chat group &lt;channel&gt; in this tab");
   WriteColorLine("AAAAAA", "\'/channels\'            - List the connect channels in this tab");
   WriteColorLine("AAAAAA", "\'/who\'                 - List the users in the active channel");
   WriteColorLine("AAAAAA", "\'/this\'                - Display the active channel");
   WriteColorLine("AAAAAA", "\'&lt;message&gt;\'            - Write a message to the active channel");
   WriteColorLine("AAAAAA", "\'/&lt;channel&gt;\'           - Switch active channels");
   WriteColorLine("AAAAAA", "\'/&lt;channel&gt; &lt;message&gt;\' - Switch active channels and write a message to the new channel");
}

bool Chat::Group::IsListeningToChannel(const QString& aChannel) const
{
   return mChannels.contains(aChannel);
}

bool Chat::Group::JoinChannel(const QString& aChannel)
{
   if (mActiveChannel == aChannel)
   {
      WriteColorLine("FF6666", "This tab is already writing to channel " + aChannel);
      return false;
   }
   mActiveChannel = aChannel;
   emit ChannelChanged(this);
   if (!mChannels.contains(aChannel))
   {
      mChannels += aChannel;
      return true;
   }
   return false;
}

bool Chat::Group::LeaveChannel(const QString& aChannel)
{
   if (!mChannels.contains(aChannel))
   {
      WriteColorLine("FF6666", "This tab is not listening to channel " + aChannel);
      return false;
   }
   mChannels.removeAll(aChannel);
   if (mChannels.isEmpty())
   {
      mActiveChannel.clear();
      emit ChannelChanged(this);
   }
   else if (mActiveChannel == aChannel)
   {
      SwitchChannel(mChannels.first());
   }

   return true;
}

bool Chat::Group::SwitchChannel(const QString& aChannel)
{
   if (mChannels.contains(aChannel) && mActiveChannel != aChannel)
   {
      mActiveChannel = aChannel;
      emit ChannelChanged(this);
      return true;
   }
   return false;
}

void Chat::Group::SaveAs(QString aFileName /*= {}*/) const
{
   if (aFileName.isEmpty())
   {
      aFileName = wkf::getSaveFileName(nullptr, "Save to file", QString(), "*.txt");
   }

   if (!aFileName.isEmpty())
   {
      QFile file(aFileName);
      if (file.open(QIODevice::WriteOnly))
      {
         file.write(document()->toPlainText().toLocal8Bit());
         file.close();
      }
      else
      {
         const QString msg = "Unable to save chat for group %1 to file %2.";
         QMessageBox::critical(nullptr, "Chat Save", msg.arg(objectName(), aFileName));
      }
   }
}

void Chat::Group::mousePressEvent(QMouseEvent* e)
{
   QString anchor = anchorAt(e->pos());
   if (!anchor.isEmpty())
   {
      QString errorMsg;
      if (!wkf::util::FollowLink(anchor, errorMsg))
      {
         WriteColorLine("FF6666", errorMsg);
      }
   }
   QPlainTextEdit::mousePressEvent(e);
}

void Chat::Group::mouseMoveEvent(QMouseEvent* e)
{
   QString anchor = anchorAt(e->pos());
   if (!anchor.isEmpty())
   {
      viewport()->setCursor(Qt::PointingHandCursor);
   }
   else
   {
      viewport()->setCursor(Qt::IBeamCursor);
   }
   QPlainTextEdit::mouseMoveEvent(e);
}
