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

#include "ChatTextChannelChat.hpp"

#include <QColor>
#include <QTime>

#include "UtRandom.hpp"
#include "WkfUtils.hpp"

Chat::TextChannelChat::TextChannelChat(const QString& aName, QObject* aParent /* = nullptr */)
   : TextChannel(aParent)
   , mName(aName)
   , mPrefix("")
   , mPostfix("")
   , mShowTimeStamps(false)
{
   SetColorFromName(aName);
}

void Chat::TextChannelChat::MessageReceived(const QString& aMessage)
{
   QString str = GetChannelPrefix() + aMessage;
   emit    TextReady(str);
}

void Chat::TextChannelChat::SetColor(const QString& aHex)
{
   mPrefix  = "<font color='#" + aHex + "'>";
   mPostfix = "</font>";
}

void Chat::TextChannelChat::HandleInput(const QString& aInput)
{
   emit UserInput(mName, wkf::util::ParseLinks(aInput));
}

void Chat::TextChannelChat::SetColorFromName(const QString& aName)
{
   QColor        c     = wkf::util::GetColor(aName);
   unsigned char red   = c.redF() * 255;
   unsigned char green = c.greenF() * 255;
   unsigned char blue  = c.blueF() * 255;
   QString       str;
   if (red < 16)
   {
      str += "0";
   }
   str += QString::number(red, 16);
   if (green < 16)
   {
      str += "0";
   }
   str += QString::number(green, 16);
   if (blue < 16)
   {
      str += "0";
   }
   str += QString::number(blue, 16);
   SetColor(str);
}

QString Chat::TextChannelChat::GetChannelPrefix() const
{
   if (mShowTimeStamps)
   {
      return "(" + QTime::currentTime().toString() + ")" + GetChannelLabel();
   }
   else
   {
      return GetChannelLabel();
   }
}

QString Chat::TextChannelChat::GetChannelLabel() const
{
   return "[" + mPrefix + mName + mPostfix + "]";
}

void Chat::TextChannelChat::SetShowTimeStamps(bool aState)
{
   mShowTimeStamps = aState;
}
