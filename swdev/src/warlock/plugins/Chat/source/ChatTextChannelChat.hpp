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

#ifndef CHATTEXTCHANNELCHAT_HPP
#define CHATTEXTCHANNELCHAT_HPP

#include <QObject>

#include "ChatTextChannel.hpp"

namespace Chat
{
class TextChannelChat : public TextChannel
{
public:
   TextChannelChat(const QString& aName, QObject* aParent = nullptr);

   bool    SupportsInput() const override { return true; }
   void    HandleInput(const QString& aInput) override;
   QString GetChannelPrefix() const;
   QString GetChannelLabel() const override;
   void    SetShowTimeStamps(bool aState) override;

   void MessageReceived(const QString& aMessage) override;

private:
   void SetColorFromName(const QString& aName);
   void SetColor(const QString& aHex);

   QString mName;
   QString mPrefix;
   QString mPostfix;
   bool    mShowTimeStamps;
};
} // namespace Chat

#endif
