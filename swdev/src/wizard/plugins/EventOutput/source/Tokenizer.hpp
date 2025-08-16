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

#ifndef EVENT_OUTPUT_TOKENIZER_HPP
#define EVENT_OUTPUT_TOKENIZER_HPP

#include <QString>
#include <QStringList>
#include <QTextCursor>

#include "CommandValuePair.hpp"
#include "UtMemory.hpp"
#include "UtOptional.hpp"

namespace wizard
{

class TextSource;

namespace EventOutput
{
namespace editor
{

class Editor;

//! Namespace for objects, functions, and variables related to the Tokenizer
namespace tokenizer
{

// defined in PluginUtil
struct Signals;

//! Represents the begin and end tag of an AFSIM input block
struct BlockTags
{
   //! The beginning of the block
   QString mBeginTag;
   //! The end of the block
   QString mEndTag;
};

//! A light-weight state machine used for tokenizing
//! @par details
//!   This state machine is used by the Tokenizer to convert a string into a list of tokens.  @see Tokenize for more
//!   information. it also has the capability to give the tokens meaning via a classification routine.  @see Classify
//!   for more information.
class StateMachine final
{
public:
   StateMachine()                         = default;
   StateMachine(const StateMachine& aSrc) = delete;
   StateMachine(StateMachine&& aSrc)      = default;
   ~StateMachine()                        = default;

   StateMachine& operator=(const StateMachine& aSrc) = delete;
   StateMachine& operator=(StateMachine&& aSrc) = default;

   //! Convenient aliases
   //@{
   using TokenList     = QVector<token::Token>;
   using TokenType     = token::Token::Type;
   using TokenMetaData = token::Token::MetaData;
   //@}

   //! Convert a string into token::CommandValuePairs
   //! @param aText is the string to convert into token::CommandValuePairs
   const TokenList& Tokenize(const QString& aText);

   //! Classifies the token::CommandValuePairs into categories
   //! @param aTokenList is the list of token::CommandValuePairs to classify
   //! @param aTags are the begin and end tags of the block
   //! @note this distinguishes tokens inside the intended block (defined by the aTags) and tokens inside an unwanted block.
   void Classify(TokenList& aTokenList, const BlockTags& aTags);

   // using protected so Doxygen can pick up on it...
protected:
   //! Denotes the state of the Tokenize state machine
   enum class State
   {
      cDEFAULT,          //!< denotes the default state
      cIDENTIFIER,       //!< denotes the state when it finds an Identifier
      cSTRING,           //!< denotes the state when it finds a String
      cSTRING_ESC,       //!< denotes the state when it finds an escaped backslash or double-quote inside a String
      cSLASH,            //!< denotes the state when it finds a forward slash or an asterisk
      cLINE_COMMENT,     //!< denotes the state when it fnds a LineComment
      cBLOCK_COMMENT,    //!< denotes the state when it finds a BlockComment
      cBLOCK_COMMENT_ESC //!< denotes the state when it finds an asterisk inside a BlockComment
   };

   //! Creates a token::CommandValuePair and appends it to the list
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   void BeginToken(const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Appends a character to the current token::CommandValuePair's text
   //! @param aCharacter is the character to append
   void PushChar(const QChar aCharacter);

   //! Determines whether or not a character is part of an identifier
   //! @param aCharacter is the character to test
   //! @return whether or not the character is part of an identifier
   bool IsPartOfIdentifier(const QChar aCharacter) const;

   //! Determines the next state from the Default state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State DefaultState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the IdentifierState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State IdentifierState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the StringState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State StringState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the StringEscState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State StringEscState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the SlashState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State SlashState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the LineCommentState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State LineCommentState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the BlockCommentState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State BlockCommentState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! Determines the next state from the BlockCommentAsteriskState state
   //! @param aNextChar is the character being processed
   //! @param aRow is the starting row
   //! @param aColumn is the starting column
   //! @param aPosition is the starting position
   //! @return the next state
   State BlockCommentEscState(const QChar aNextChar, const size_t aRow, const size_t aColumn, const size_t aPosition);

   //! The list of token::Tokens
   TokenList mTokens;
};

//! A light-weight tokenizer for csv_event_output and event_output
//! @par details
//!   This is the brains of the back-end of the EventOutput::Dialog.
//!  it tokenizes a block of input text into tokens.
//!  these tokens are further processed into command-value pairs (CVPs).
//!  those CVPs are modified by the front-end widgets.
//! @par the Tokenizer state machine
//!   The Tokenizer uses a state machine under-the-hood to tokenize the string.
//!   the reason for this is that the AFSIM input file can get complicated enough that other methods fail for finding
//!   the block range. a naiive string split will not work because other blocks could use the begin and/or end tag as a
//!   name. regex will also fail because it does not take comments into consideration.
//! @see tokenizer::StateMachine for more information.
class Tokenizer
{
public:
   using TokenType     = token::Token::Type;
   using TokenMetaData = token::Token::MetaData;

   //! Constructs a Tokenizer
   //! @param aTags are the begin and end tags of the block
   Tokenizer(BlockTags aTags);
   //! Constructs a Tokenizer
   //! @param aSourcePtr is the wizard::TextSource from which to refer to the QTextDocument
   //! @param aTags are the begin and end tags of the block
   Tokenizer(TextSource* aSourcePtr, BlockTags aTags);
   //! Destructs a Tokenizer
   ~Tokenizer() = default;

   //! Set the wizard::TextSource reference
   //! @param aSourcePtr is the wizard::TextSource from which to refer
   void SetSource(TextSource* aSourcePtr);
   //! Get the wizard::TextSource reference
   //! @return the wizard::TextSource to which the Tokenizer refers
   const TextSource* GetSource() const;
   //! Get the Qt-based range of the block
   //! @return a cursor containing the range of the block
   QTextCursor GetRangeFromQtDocument() const;

   //! @name methods related to command-value pairs
   //@{

   //! Insert a setting command-value pair into the list
   //! @param aSetting is the name (command) of the setting
   //! @param aValue is the value of the setting
   void InsertSetting(const QString& aSetting, const QString& aValue);
   //! Remove a setting command-value pair from the list
   //! @param aSetting is the name (command) of the setting
   void RemoveSetting(const QString& aSetting);
   //! Set the value of a setting command-value pair
   //! @param aSetting is the name (command) of the setting
   //! @param aValue is the new value of the setting
   void SetSetting(const QString& aSetting, const QString& aValue);
   //! Get the command-value pair associated with a particular setting name
   //! @param aSetting the name (command) of the setting
   //! @return a pointer to the command-value pair associated with the setting name (immutable)
   const token::CommandValuePair* GetSetting(const QString& aSetting) const;
   //! Set the setting list
   //! @param aSettingList is the new setting list
   void SetSettings(const QMap<token::CommandString, token::SettingCVP>& aSettingList);
   //! Get the setting list
   //! @return the setting list
   const QMap<token::CommandString, token::SettingCVP>& GetSettings() const;
   //! Validate the setting command-value pairs based on the available settings from the WsfParser
   //! @param aValidSettings is the list of available settings acquired from the WsfParser
   //! @note this method will remove any setting command-value pair whose name (command) is not in the available settings
   //! @see setting::Widget for more information
   void ValidateSettings(const QStringList& aValidSettings);
   //! Insert an event command-value pair into the list
   //! @param aEvent is the name (command) of the event
   //! @param aValue is the value of the event
   void InsertEvent(const QString& aEvent, const QString& aValue);
   //! Remove an event command-value pair from the list
   //! @param aEvent is the name (command) of the event
   void RemoveEvent(const QString& aEvent);
   //! Set the value of an event command-value pair
   //! @param aEvent is the name (command) of the event
   //! @param aValue is the new value of the event
   void SetEvent(const QString& aEvent, const QString& aValue);
   //! Get the command-value pair associated with a particular event name
   //! @param aEvent the name (command) of the event
   //! @return a pointer to the command-value pair associated with the event name (immutable)
   const token::CommandValuePair* GetEvent(const QString& aEvent) const;
   //! Set the event list
   //! @param aEventList is the new event list
   void SetEvents(const QMap<token::CommandString, token::EventCVP>& aEventList);
   //! Get the event list
   //! @return the event list
   const QMap<token::CommandString, token::EventCVP>& GetEvents() const;
   //! Validate the event command-value pairs based on the available events from the WsfParser
   //! @param aValidEvents is the list of available events acquired from the WsfParser
   //! @note this method will remove any event command-value pair whose name (command) is not in the available events
   //! @see event::EventsGroupBoxWidget for more information
   void ValidateEvents(const QStringList& aValidEvents);

   //@}

   //! Set the "need to save" property
   //! @param aNeedToSave denotes whether or not to build the "cache" string
   void SetNeedToSave(bool aNeedToSave);
   //! Get the "need to save" property
   //! @return whether or not to build the "cache" string
   bool NeedToSave() const;

   //! Connects to an editor::Editor
   //! @param aEditor is the editor::Editor to which to connect
   //! @return whether or not the connection succeeds
   //! @note a Tokenizer can connect only to one editor::Editor once
   bool ConnectToEditor(const Editor* aEditor);
   //! Create the command-value pairs from token::CommandValuePairs
   void CreateCVPsFromTokens();
   //! Create the block string from the command-value pairs
   void CreateStringFromCVPs();
   //! Get the block "cache" string
   //! @return the block "cache" string
   QString GetCache() const;
   //! Determines whether or not there is a "cache" string available
   //! @return wther or not there is a "cache" string avaiable
   bool IsCached() const;

private:
   //! Finds the block range in the token::CommandValuePair list
   void FindRangeInQtDocument();

   //! Convenient alias for a token::CommandValuePair list
   using TokenList = QVector<token::Token>;

   //! Get the iterator to the beginning of the block
   //! @return the iterator to the beginning of the block
   TokenList::iterator BeginEventOutput();
   //! Get the iterator to the end of the block
   //! @return the iterator to the end of the block
   TokenList::iterator EndEventOutput();

   //! The owning editor::Editor
   const Editor* mEditorPtr{nullptr};
   //! The begin and end tags of the block
   BlockTags mTags;
   //! The wizard::TextSource reference
   //! @note this does not need the actual wizard::TextSource, just the QTextDocument to which it refers
   TextSource* mSource{nullptr};
   //! The Qt-based block range
   QTextCursor mQtRange;
   //! The block "cache" string
   ut::optional<QString> mBlockCache;
   //! The list of token::CommandValuePairs
   TokenList mTokens;
   //! The setting command-value pairs
   QMap<token::CommandString, token::SettingCVP> mSettings;
   //! The event command-value pairs
   QMap<token::CommandString, token::EventCVP> mEvents;
   //! The "need to save" property
   bool mNeedToSave{true};
};

} // namespace tokenizer
} // namespace editor
} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_TOKENIZER_HPP
