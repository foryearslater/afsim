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

#ifndef MAP_UTILS_MESSAGE_HPP
#define MAP_UTILS_MESSAGE_HPP

#include <array>

#include <QMessageBox>
#include <QString>

#include "UtLogMessage.hpp"
#include "UtMemory.hpp"

namespace MapUtils
{

//! @brief a "message box" type of message
//! @par details
//!    This class provides a convenient way to display messages to the user in AFSIM Wizard,
//!    as there is no convenient way to print to the console.
//! @note it creates a QMessageBox with the message information when it is displayed
class Message final
{
public:
   //! @brief QString version of ut::log::Message
   struct Body final
   {
      QString           mData; //!< denotes the main part of the Message::Body
      std::vector<Body> mNotes; //!< denotes any additional information that supplements the main part of the Message::Body
   };

   //! @brief the default, three-space indent for formatting notes for output
   //! @note the three spaces is taken from the <a
   //! href="../../documentation/html/docs/developer/coding_standards.html#id39">AFSIM Coding Standards</a>
   static const QString cDEFAULT_INDENT;

   //! @brief Message::Level is the same as QMessageBox::Icon
   using Level = QMessageBox::Icon;

   //! @brief constructs a message at the given level with the given title and message
   //! @param aMessageLevel is the severity of the message (this cannot be changed)
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a QString)
   //! @param aMessageFormat is the format of the text for the message body
   //! @param aShowType is whether or not to show the type of the message in the body
   //! @note the aMessage parameter is just the main part of the Message::Body.
   //!    if you want to add notes, then please use the AddNote function.
   explicit Message(const Level          aMessageLevel,
                    const QString&       aMessageTitle,
                    const QString&       aMessageBody,
                    const Qt::TextFormat aMessageFormat,
                    const bool           aShowType) noexcept;

   //! @brief constructs a message at the given level with the given title and message
   //! @param aMessageLevel is the severity of the message (this cannot be changed)
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a Message::Body)
   //! @param aMessageFormat is the format of the text for the message body
   //! @param aShowType is whether or not to show the type of the message in the body
   explicit Message(const Level          aMessageLevel,
                    const QString&       aMessageTitle,
                    const Body&          aMessageBody,
                    const Qt::TextFormat aMessageFormat,
                    const bool           aShowType) noexcept;

   //! @brief constructs a message with the given title and message
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a ut::log::Message)
   //! @param aMessageFormat is the format of the text for the message body
   //! @param aShowType is whether or not to show the type of the message in the body
   //! @note the severity of the message is contained in the ut::log::Message's type list
   explicit Message(const QString&          aMessageTitle,
                    const ut::log::Message& aMessageBody,
                    const Qt::TextFormat    aMessageFormat,
                    const bool              aShowType) noexcept;

   //! @name constructors that have text format Qt::TextFormat::AutoText and show the type when displayed
   //@{

   //! @brief constructs a message at the given level with the given title and body
   //! @param aMessageLevel is the severity of the message (this cannot be changed)
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a QString)
   explicit Message(const Level aMessageLevel, const QString& aMessageTitle, const QString& aMessageBody) noexcept;

   //! @brief constructs a message at the given level with the given title and body
   //! @param aMessageLevel is the severity of the message (this cannot be changed)
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a Message::Body)
   explicit Message(const Level aMessageLevel, const QString& aMessageTitle, const Body& aMessageBody) noexcept;

   //! @brief constructs a message with the given title and body
   //! @param aMessageTitle is the title of the message
   //! @param aMessageBody is the body of the message (as a ut::log::Message)
   explicit Message(const QString& aMessageTitle, const ut::log::Message& aMessageBody) noexcept;

   //@}

   //! @note does not make sense to allow this
   Message()                          = delete;
   Message(const Message& /* aSrc */) = default;
   Message(Message&& /* aSrc */)      = default;
   ~Message()                         = default;

   Message& operator=(const Message& /* aSrc */) = delete;
   Message& operator=(Message&& /* aSrc */) = delete;

   //! @brief accesses the severity of the message
   //! @return the severity of the corresponding message
   Level GetLevel() const noexcept { return mMessageLevel; }

   //! @brief sets the title of the message
   //! @param aMessageTitle is the severity with which to set the corresponding message
   void SetTitle(const QString& aMessageTitle) noexcept { mMessageTitle = aMessageTitle; }

   //! @brief accesses the title of the message
   //! @return the title of the corresponding message
   const QString& GetTitle() const noexcept { return mMessageTitle; }

   //! @brief sets the title of the message
   //! @param aMessageBody is the body to set the corresponding message (as a Message::Body)
   void SetBody(const Body& aMessageBody) noexcept { mMessageBody = aMessageBody; }

   //! @brief sets the title of the message
   //! @param aMessageBody is the body to set the corresponding message (as a QString)
   void SetBody(const QString& aMessageBody) noexcept { mMessageBody = StringToBody(aMessageBody); }

   //! @brief accesses the body of the message
   //! @return the body of the corresponding message (as a Message::Body)
   const Body& GetBody() const noexcept { return mMessageBody; }

   //! @brief accesses the body of the message
   //! @return the body of the corresponding message (as a ut::log::Message)
   ut::log::Message GetBodyAsLogMessage() const noexcept;

   //! @brief accesses the body of the message
   //! @return the body of the corresponding message (as a QString)
   QString GetBodyAsString() const noexcept { return BodyToString(mMessageBody); }

   //! @brief sets the format of the text for the message body
   //! @param aMessageFormat is the text format for the corresponding message
   //! @note needed for hyper-links in QMessageBox
   void SetTextFormat(const Qt::TextFormat aMessageFormat) noexcept { mMessageFormat = aMessageFormat; }

   //! @brief accesses the format of the text for the message body
   //! @return the format of the text for the message body
   //! @note needed for hyper-links in QMessageBox
   Qt::TextFormat GetTextFormat() const noexcept { return mMessageFormat; }

   //! @brief accesses the show type control variable
   //! @param aShowType is the whether or not to show the type of the corresponding message
   //! @return whether or not it will show type when displayed
   void SetShowType(const bool aShowType) { mShowType = aShowType; }

   //! @brief accesses the show type control variable
   //! @return whether or not it will show type when displayed
   bool GetShowType() const noexcept { return mShowType; }

   //! @brief displays the message on the screen
   void Display() const noexcept;

   //! @name conversion functions
   //@{

   //! @brief converts a Message::Body into a single QString
   //! @param aBody is the Message::Body to convert
   //! @return a QString as the result of the conversion
   static QString BodyToString(const Body& aBody) noexcept;

   //! @brief converts a single QString into a Message::Body
   //! @param aString is the QString to convert
   //! @return a Message::Body as the result of the conversion
   static Body StringToBody(const QString& aString) noexcept;

   //! @brief converts a Message::Body into a ut::log::Message
   //! @param aBody is the Message::Body to convert
   //! @return a ut::log::Message as the result of the conversion
   static ut::log::Message BodyToLogMessage(const Body& aBody) noexcept;

   //! @brief converts a ut::log::Message into a Message::Body
   //! @param aMessage is the ut::log::Message to convert
   //! @return a Message::Body as the result of the conversion
   static Body LogMessageToBody(const ut::log::Message& aMessage) noexcept;

   //! @brief converts a Message::Level into a QString
   //! @param aLevel is the Message::Level to convert
   //! @return a QString as the result of the conversion
   static QString LevelToString(const Level aLevel) noexcept;

   //! @brief converts a QString into a Message::Level
   //! @param aString is the QString to convert
   //! @return a Message::Level as the result of the conversion
   static Level StringToLevel(const QString& aString) noexcept;

   //! @brief converts a Message::Level into a ut::log::TypeList with one element
   //! @param aLevel is the Message::Level to convert
   //! @return a ut::log::TypeList as the result of the conversion
   static ut::log::TypeList LevelToType(const Level aLevel) noexcept;

   //! @brief converts a ut::log::TypeList with one element into a Message::Level
   //! @param aType is the ut::log::TypeList to convert
   //! @return a Level as the result of the conversion
   static Level TypeToLevel(const ut::log::TypeList& aType) noexcept;

   //@}

protected:
   //! @brief denotes the severity of the message
   //! @note the level also represents the icon for the QMessageBox
   const Level mMessageLevel;

private:
   //! @brief private helper method to execute the recursive parsing for StringToBody
   //! @param aBody is the Message::Body to convert
   //! @param aIndent is the number of indents to prefix the notes of the current Message::Body
   //! @return a QString as the result of the conversion
   static QString ParseBodyToStringP(const Body& aBody, const int aIndent);

   //! @name helper methods for StringToBody
   //@{

   //! @brief private helper method to execute the recursive parsing for StringToBody
   //! @param aString is the QString to convert
   //! @param aIndent current indent level
   //! @return a Message::Body as the result of the conversion
   static Body ParseStringToBodyP(QString& aString, const int aIndent);

   //! @brief gets the indent level of the current line
   //! @param aString is the source string containing the current line from which to indent level
   //! @return the indent level (of the current line)
   static int GetIndentOfCurrentLineP(const QString& aString);

   //! @brief pops the current line off the source string and returns it
   //! @param aString is the source string
   //! @param aIndent is the current line indent level
   //! @return the current line (popped off the source string)
   static QString PopCurrentLineP(QString& aString, const int aIndent);

   //@}

   //! @brief denotes the title of the message
   //! @note this is displayed in the title bar of the message box
   QString mMessageTitle;

   //! @brief denotes the body of the message
   //! @note this is displayed in the main area of the message box
   Body mMessageBody{};

   //! @brief denotes the format of the text for the message body
   Qt::TextFormat mMessageFormat;

   //! @brief denotes whether or not to show the type of message in the body
   bool mShowType;
};

//! @name conditional operators for MapUtils::Message
//@{

bool operator==(const Message::Body& aLhs, const Message::Body& aRhs);
bool operator!=(const Message::Body& aLhs, const Message::Body& aRhs);
bool operator==(const Message& aLhs, const Message& aRhs);
bool operator!=(const Message& aLhs, const Message& aRhs);

//@}

} // namespace MapUtils

#endif // MAP_UTILS_MESSAGE_HPP
