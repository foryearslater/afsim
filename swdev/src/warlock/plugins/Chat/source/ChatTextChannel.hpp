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

#ifndef CHATTEXTCHANNEL_HPP
#define CHATTEXTCHANNEL_HPP

#include <QObject>

#include "comment/WkfCommentForwardingService.hpp"

namespace Chat
{
class TextChannel : public QObject
{
   Q_OBJECT
public:
   TextChannel(QObject* aParent = nullptr)
      : QObject(aParent)
   {
      wkf::CommentForwardingService* forwarder = wkf::CommentForwardingService::Request();
      if (forwarder != nullptr)
      {
         connect(forwarder, &wkf::CommentForwardingService::ClearComments, this, &Chat::TextChannel::ClearComments);
      }
   }

   ~TextChannel() override { wkf::CommentForwardingService::Release(); }

   virtual void    ClearBacklog() {}
   virtual bool    SupportsInput() const { return false; }
   virtual void    HandleInput(const QString& aInput) {}
   virtual QString GetChannelLabel() const { return "<b>SYSTEM</b>"; }
   virtual void    SetShowTimeStamps(bool aState) {}
   virtual void    Save() {}
   virtual void    Reload() {}

signals:
   void TextReady(const QString& aText);
   void ClearComments();
   void UserInput(const QString& aChannelName, const QString& aText);

public slots:
   virtual void MessageReceived(const QString& aMessage)
   {
      emit TextReady(GetChannelLabel() + "<b>: " + aMessage + "</b><br>");
   }
};
} // namespace Chat

#endif
