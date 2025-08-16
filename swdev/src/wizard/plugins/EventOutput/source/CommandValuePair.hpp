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

#ifndef EVENT_OUTPUT_TOKENS_HPP
#define EVENT_OUTPUT_TOKENS_HPP

#include <QString>
#include <QStringList>

namespace wizard
{
namespace EventOutput
{
namespace editor
{

//! Namespace for objects, functions, and variables related to Tokens and command-value pairs (CVPs)
namespace token
{

//! Represents a token
struct Token
{
   //! Represents the token type
   enum class Type
   {
      cUNCLASSIFIED,       //!< denotes an unclassified token
      cBEGIN_BLOCK,        //!< denotes the start of an unwanted block
      cEND_BLOCK,          //!< denotes the end of an unwanted block
      cBEGIN_EVENT_OUTPUT, //!< denotes the beginning of a csv_event_output and event_output block
      cEND_EVENT_OUTPUT,   //!< denotes the end of a csv_event_output and event_output block
      cCOMMAND,            //!< denotes a property name (command)
      cVALUE,              //!< denotes a property value
      cLINE_COMMENT,       //!< denotes a line comment
      cBLOCK_COMMENT       //!< denotes a block comment
   };

   //! Represents the metadata of a token
   struct MetaData
   {
      //! The token Type
      Type mType;
      //! The starting position
      size_t mPosition;
      //! The starting row
      size_t mRow;
      //! The starting column
      size_t mColumn;
   };

   //! The token text
   QString mText;
   //! The token metadata
   MetaData mMetadata;
};

//! A light-weight wrapper around a QString
//! @note this is to provide case-insensitive key comparison for command strings inside a map
struct CommandString
{
   QString mText;

   bool operator<(const CommandString& aRhs) const
   {
      if (mText == aRhs.mText)
      {
         return false;
      }
      else if (mText == "all")
      {
         return true;
      }
      else if (aRhs.mText == "all")
      {
         return false;
      }
      return mText.compare(aRhs.mText, Qt::CaseInsensitive) < 0;
   }
};

//! Represents a command-value pair (CVP)
//! @par details
//!   This is the base struct for a pair of strings called a CVP.
//!   the command string is the name of a property of interest.
//!   the value string the property's value.
//!   for csv_event_output and event_output, there are two types of CVPs:  setting and event.
//! @note this hierarchy uses the Non-virtual Interface pattern
struct CommandValuePair
{
   //! Constructs a CommandValuePair
   CommandValuePair() = default;
   //! Constructs a CommandValuePair
   //! @param aCommand is the name of the property
   //! @param aValue is the value of the property
   CommandValuePair(QString aCommand, QString aValue);
   //! Constructs a CommandValuePair
   //! @param aCommandValue is a two element string list:  { command, value }
   //! @note doing copy-move here because QStringList should be small enough to do it
   CommandValuePair(QStringList aCommandValue);
   //! Destructs a CommandValuePair
   virtual ~CommandValuePair() = default;

   //! @name the public, non-virtual interface
   //! @note simply calls the private, virtual interface
   //@{

   //! Set the name of the property
   //! @param aCommand is the name of the property
   void SetCommand(const QString& aCommand) { mCommand = aCommand; }
   //! Get the name of the property
   //! @return the name of the property
   const QString& GetCommand() const { return mCommand; }
   //! Set the value of the property
   //! @param aValue is the value of the property
   void SetValue(const QString& aValue) { mValue = aValue; }
   //! Get the value of the property
   //! @return the value of the property
   const QString& GetValue() const { return mValue; }

   //! Determines whether or not the CVP is valid
   //! @return whether or not the CVP is valid
   bool IsValid() const { return !mCommand.isEmpty(); }
   //! Get the string representation of the CVP
   //! @return the string representation of the CVP
   QString ToString() const { return ToStringP(); }

   //@}

protected:
   //! The name of the property
   QString mCommand;
   //! The value of the property
   QString mValue;

private:
   //! @name the private, virtual interface
   //@{

   //! Get the string representation of the CVP
   //! @return the string representation of the CVP
   virtual QString ToStringP() const = 0;

   //@}
};

//! Represents a setting command-value pair (CVP)
struct SettingCVP final : public CommandValuePair
{
   //! Inheriting the parent constructors
   using CommandValuePair::CommandValuePair;

private:
   //! @name the private, virtual interface
   //@{

   //! Get the string representation of the CVP
   //! @return the string representation of the CVP
   QString ToStringP() const override;

   //@}
};

//! Represents a setting command-value pair (CVP)
//! @par event CVP storage
//!   There is a caveat to storing event CVPs in a map.
//!   the actual command string is disable/enable and the actual value string is unique.
//!   therefore, the map would have two keys with multiple values.
//!   it should have a one-to-one mapping, instead of that, so flip the command and value.
//!   this causes a slight headache, but it is better.
struct EventCVP final : public CommandValuePair
{
   //! Inheriting the parent constructors
   using CommandValuePair::CommandValuePair;

private:
   //! @name the private, virtual interface
   //@{

   //! Get the string representation of the CVP
   //! @return the string representation of the CVP
   QString ToStringP() const override;

   //@}
};

} // namespace token
} // namespace editor
} // namespace EventOutput
} // namespace wizard

#endif
