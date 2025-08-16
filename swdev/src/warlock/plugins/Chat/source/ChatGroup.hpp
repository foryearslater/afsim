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

#ifndef CHATGROUP_HPP
#define CHATGROUP_HPP

#include <QPlainTextEdit>
#include <QTextBrowser>

namespace Chat
{
//! A Group is a widget that displays the output of one-or-more channels.
//! When information is written to a channel, the Plugin determines which
//!    Groups are listening to the channel and adds text to them accordingly.
//! In addition to channel output, there is system output, which is written to every group.
//! System output includes network errors and user rename announcements.
//! Lastly, certain commands and error messages may be written to a specific Group unassociated with any channel.
class Group : public QPlainTextEdit
{
   Q_OBJECT

public:
   Group(QWidget* aParent, const QString& aName, Group* aGroup = nullptr);

   void contextMenuEvent(QContextMenuEvent* e) override;

   QString GetActiveChannelLabel() const;

   //! Writes a line of HTML to the Group.
   void WriteHtml(const QString& aHtml);
   //! Helper method for writing a line in a specific color.
   //! Color format should be "RRGGBB", e.g. "FF6666"
   void WriteColorLine(const QString& aColor, const QString& aHtml);

   //! Called by the /channels command.
   void WriteChannelList();
   //! Called by the /who command.
   void WriteUserList(const QStringList& aUsers);
   //! Called by the /help command, or when the "?" button is clicked.
   void WriteHelp();

   const QStringList& GetChannelList() const { return mChannels; }
   const QString&     GetActiveChannel() const { return mActiveChannel; }
   bool               IsListeningToChannel(const QString& aChannel) const;
   //! Joins the channel and sets the active channel.
   //! Returns true on successful join.
   //! Returns false if only the active channel changes.
   bool JoinChannel(const QString& aChannel);
   //! Leaves the channel. Returns true on success.
   bool LeaveChannel(const QString& aChannel);
   //! Switches to aChannel. Returns true on success.
   bool SwitchChannel(const QString& aChannel);

   //! If aFileName is empty, prompts the user.
   //! Intentionally passing by value.
   void SaveAs(QString aFileName = {}) const;

signals:
   //! Emitted when text is added so the dock widget can update the tab name.
   void TextAdded(Group* aThis);

   //! Emitted when JoinChannel or LeaveChannel causes the active channel to change.
   void ChannelChanged(Group* aThis);

private:
   void mousePressEvent(QMouseEvent* e) override;
   void mouseMoveEvent(QMouseEvent* e) override;

   QStringList mChannels;
   QString     mActiveChannel;
};
} // namespace Chat

#endif
