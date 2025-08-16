// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Message.hpp"

#include "UtStringUtil.hpp"

namespace MapUtils
{

const QString Message::cDEFAULT_INDENT{"   "};

Message::Message(const Level          aMessageBodyLevel,
                 const QString&       aMessageBodyTitle,
                 const QString&       aMessageBody,
                 const Qt::TextFormat aMessageFormat,
                 const bool           aShowType) noexcept
   : mMessageLevel(aMessageBodyLevel)
   , mMessageTitle(aMessageBodyTitle)
   , mMessageFormat(aMessageFormat)
   , mShowType(aShowType)
{
   mMessageBody = StringToBody(aMessageBody);
}

Message::Message(const Level          aMessageBodyLevel,
                 const QString&       aMessageBodyTitle,
                 const Body&          aMessageBody,
                 const Qt::TextFormat aMessageFormat,
                 const bool           aShowType) noexcept
   : mMessageLevel(aMessageBodyLevel)
   , mMessageTitle(aMessageBodyTitle)
   , mMessageBody(aMessageBody)
   , mMessageFormat(aMessageFormat)
   , mShowType(aShowType)
{
}

Message::Message(const QString&          aMessageBodyTitle,
                 const ut::log::Message& aMessageBody,
                 const Qt::TextFormat    aMessageFormat,
                 const bool              aShowType) noexcept
   : mMessageLevel(TypeToLevel(aMessageBody.mTypes))
   , mMessageTitle(aMessageBodyTitle)
   , mMessageFormat(aMessageFormat)
   , mShowType(aShowType)
{
   mMessageBody = LogMessageToBody(aMessageBody);
}

Message::Message(const Level aMessageBodyLevel, const QString& aMessageBodyTitle, const QString& aMessageBody) noexcept
   : Message(aMessageBodyLevel, aMessageBodyTitle, aMessageBody, Qt::TextFormat::AutoText, true)
{
}

Message::Message(const Level aMessageBodyLevel, const QString& aMessageBodyTitle, const Body& aMessageBody) noexcept
   : Message(aMessageBodyLevel, aMessageBodyTitle, aMessageBody, Qt::TextFormat::AutoText, true)
{
}

Message::Message(const QString& aMessageBodyTitle, const ut::log::Message& aMessageBody) noexcept
   : Message(aMessageBodyTitle, aMessageBody, Qt::TextFormat::AutoText, true)
{
}

ut::log::Message Message::GetBodyAsLogMessage() const noexcept
{
   ut::log::Message body = BodyToLogMessage(mMessageBody);
   body.mTypes.Insert(LevelToType(mMessageLevel));
   return body;
}

void Message::Display() const noexcept
{
   // add the type to the string followed by a colon
   QString message;
   if (mShowType)
   {
      message += LevelToString(mMessageLevel) + ":\n";
   }
   message += BodyToString(mMessageBody);
   // create a message box with the corresponding icon, title, and body
   QMessageBox dialog(mMessageLevel, mMessageTitle, message);
   dialog.setTextFormat(mMessageFormat);
   // show it on the screen
   dialog.exec();
}

QString Message::BodyToString(const Body& aBody) noexcept
{
   return ParseBodyToStringP(aBody, 1);
}

Message::Body Message::StringToBody(const QString& aString) noexcept
{
   QString temp = aString;
   return ParseStringToBodyP(temp, 0);
}

ut::log::Message Message::BodyToLogMessage(const Body& aBody) noexcept
{
   ut::log::Message messageBody;
   // add the first string as the main part of the message
   messageBody.mData = aBody.mData.toStdString();
   // add succeeding strings as notes to the message (recursive)
   for (auto iter : aBody.mNotes)
   {
      messageBody.mNotes.push_back(BodyToLogMessage(iter));
   }
   return messageBody;
}

Message::Body Message::LogMessageToBody(const ut::log::Message& aMessageBody) noexcept
{
   Body messageBody;
   // add the first string as the main part of the message
   messageBody.mData = QString::fromStdString(aMessageBody.mData);
   // add succeeding strings as notes to the message (recursive)
   for (auto iter : aMessageBody.mNotes)
   {
      messageBody.mNotes.push_back(LogMessageToBody(iter));
   }
   return messageBody;
}

QString Message::LevelToString(const Level aMessageBodyLevel) noexcept
{
   // find the Level of the message and return the corresponding string
   switch (aMessageBodyLevel)
   {
   case Level::Critical:
      return "error";
   case Level::Warning:
      return "warning";
   case Level::Information:
      return "info";
   case Level::Question:
      return "question";
   default:
      return "";
   }
}

Message::Level Message::StringToLevel(const QString& aString) noexcept
{
   // find the string and return the corresponding Level
   if (aString == "error")
   {
      return Level::Critical;
   }
   else if (aString == "warning")
   {
      return Level::Warning;
   }
   else if (aString == "info")
   {
      return Level::Information;
   }
   else if (aString == "question")
   {
      return Level::Question;
   }
   else
   {
      return Level::NoIcon;
   }
}

ut::log::TypeList Message::LevelToType(const Message::Level aMessageBodyLevel) noexcept
{
   // find the Level of the message and return the corresponding type of message
   switch (aMessageBodyLevel)
   {
   case Level::Critical:
      return ut::log::Message::Error();
   case Level::Warning:
      return ut::log::Message::Warning();
   case Level::Information:
      return ut::log::Message::Info();
   case Level::Question:
      return "question";
   default:
      return "";
   }
}

Message::Level Message::TypeToLevel(const ut::log::TypeList& aType) noexcept
{
   // find the type of message and return the corresponding Level
   if (aType.Contains(ut::log::Message::Error()))
   {
      return Level::Critical;
   }
   else if (aType.Contains(ut::log::Message::Warning()))
   {
      return Level::Warning;
   }
   else if (aType.Contains(ut::log::Message::Info()))
   {
      return Level::Information;
   }
   else if (aType.Contains("question"))
   {
      return Level::Question;
   }
   else
   {
      return Level::NoIcon;
   }
}

QString Message::ParseBodyToStringP(const Body& aBody, const int aIndent)
{
   QString messageBody;
   // add the main part of the message to the string
   if (!aBody.mData.isEmpty())
   {
      messageBody += aBody.mData;
      // add notes to the string, prefixed with newline and indent
      for (const auto& iter : aBody.mNotes)
      {
         // append the newline character
         messageBody += '\n';
         // append the appropriate indent
         for (auto iter = 0; iter < aIndent; ++iter)
         {
            messageBody += cDEFAULT_INDENT;
         }
         // recursively append the note
         messageBody += ParseBodyToStringP(iter, aIndent + 1);
      }
   }
   return messageBody;
}

Message::Body Message::ParseStringToBodyP(QString& aString, const int aIndent)
{
   Body messageBody;
   // acquire the current indent of the string
   int myIndent = GetIndentOfCurrentLineP(aString);
   // ensure that the indent corresponds to a main part
   if (myIndent == aIndent)
   {
      // acquire the current line from the string
      QString myString = PopCurrentLineP(aString, myIndent);
      // store the current line as the main part of the body
      messageBody.mData = myString;
      // acquire the next indent of the string
      myIndent = GetIndentOfCurrentLineP(aString);
      // ensure that the indent corresponds to a sub-body
      // continue to parse the sub-bodies with the same indent
      while (myIndent == aIndent + 1)
      {
         // recursively parse each sub-body
         Body lowerMessageBody = ParseStringToBodyP(aString, aIndent + 1);
         // add the sub-body as a note to the current body
         messageBody.mNotes.push_back(lowerMessageBody);
         // acquire the next indent of the string
         myIndent = GetIndentOfCurrentLineP(aString);
      }
   }
   return messageBody;
}

int Message::GetIndentOfCurrentLineP(const QString& aString)
{
   // split the string at the newline character
   auto stringSplit = aString.split("\n");
   // ensure the current line has text
   if (!stringSplit[0].isEmpty())
   {
      // split the string by the indent
      auto lineSplit = stringSplit[0].split(Message::cDEFAULT_INDENT);
      // return the number of indents
      return lineSplit.size() - 1;
   }
   // no more text to parse
   return -1;
}

QString Message::PopCurrentLineP(QString& aString, const int aIndent)
{
   // split the string at the newline character
   auto stringSplit = aString.split("\n");
   // save the current string to prevent deletion
   QString myString = stringSplit[0];
   // pop the current line off the string
   stringSplit.erase(stringSplit.begin());
   aString = stringSplit.join("\n");
   // acquire the indent of the current line
   int myIndent = GetIndentOfCurrentLineP(myString);
   // ensure the indents match
   if (myIndent == aIndent)
   {
      // split the string by the indent
      auto stringTrim = myString.split(cDEFAULT_INDENT);
      // return the string with no indents
      return stringTrim[stringTrim.size() - 1];
   }
   // return empty string
   return "";
}

bool operator==(const Message::Body& aLhs, const Message::Body& aRhs)
{
   return (aLhs.mData == aRhs.mData) && (aLhs.mNotes == aRhs.mNotes);
}

bool operator!=(const Message::Body& aLhs, const Message::Body& aRhs)
{
   return !(aLhs == aRhs);
}

bool operator==(const Message& aLhs, const Message& aRhs)
{
   return (aLhs.GetLevel() == aRhs.GetLevel()) && (aLhs.GetTitle() == aRhs.GetTitle()) &&
          (aLhs.GetBody() == aRhs.GetBody()) && (aLhs.GetTextFormat() == aRhs.GetTextFormat()) &&
          (aLhs.GetShowType() == aRhs.GetShowType());
}

bool operator!=(const Message& aLhs, const Message& aRhs)
{
   return !(aLhs == aRhs);
}

} // namespace MapUtils
