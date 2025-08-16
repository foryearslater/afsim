// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef MODELIMPORT_PARSERLITE_HPP
#define MODELIMPORT_PARSERLITE_HPP

#include <fstream>
#include <list>
#include <string>

namespace ParserLite
{
//! Represents a string taken from a file.
//! Contains meta-data about the row and column it was taken from.
struct Token
{
   std::string mText;       //!< The text of the token.
   int         mRow    = 0; //!< What row the token starts on.
   int         mColumn = 0; //!< What column the token starts on.
};

//! Constructs a list of Tokens for use in a light parser.
//! "include" and "include_once" statements are not followed.
//! Comments and whitespace are automatically ignored.
class FileInput
{
public:
   using iterator = typename std::list<Token>::const_iterator;

   //! Constructs an empty list of tokens
   FileInput() = default;


   //! Opens the file indicated by aPath and generates a list of tokens from its contents.
   //! Read entire file on construction.
   //! @param aPath This is the path to the file to read.
   //! @note If the file could not be opened, FileInput will be empty.
   explicit FileInput(const std::string& aPath);

   FileInput(const FileInput&) = delete;
   FileInput(FileInput&&)      = default;

   FileInput& operator=(const FileInput&) = delete;
   FileInput& operator=(FileInput&&) = default;

   //! Returns true if the file is empty.
   //! Can be used to check if file was successfully opened.
   bool IsEmpty() const noexcept;

   //! Provides an iterator to the beginning of the FileInput.
   //! Lowercase for STL compatibility.
   iterator begin() const noexcept;

   //! Provides an iterator to the end of the FileInput.
   //! Lowercase for STL compatibility.
   iterator end() const noexcept;

private:
   std::list<Token> mInputStings;

   //! Retreives the next token from the file.
   //! Automatically ignores white space and comments.
   //! @param aStream This is the stream to read from.
   //! @param aToken An output parameter for the token being read.
   //! @param aRow The current row from in the file.
   //! @param aColumn The current column from in the file.
   //! @returns True if a token was successfully read.
   bool GetNextToken(std::istream& aStream, Token& aToken, int& aRow, int& aColumn);
};

//! Provides a view into a FileInput.
//! Changes to the FileInput may invalidate Inputs referencing it.
//! Safe and cheap to make copies.
class Input
{
public:
   using iterator = typename FileInput::iterator;

   friend class BlockInput;

   Input() = delete;

   //! Sets up Input to read from aFile.
   //! FileInput must not be destroyed or altered while Input is in use.
   //! @param aFile This is the FileInput to read from.
   //! @note Compilation will fail if given a rvalue FileInput.
   explicit Input(const FileInput& aFile);

   Input(const FileInput&&) = delete; // Forces reference to bind to lvalue

   //! Returns true if at the beginning of the Input.
   bool AtBeginning() const noexcept;

   //! Returns true if at the end of the Input.
   bool AtEnd() const noexcept;

   //! Returns to the beginning of the Input.
   void SeekBeginning() noexcept;

   //! Goes to the end of the Input.
   void SeekEnd() noexcept;

   //! Moves ahead a specified number of tokens in the Input.
   //! Will cease moving forward if at the end of the Input.
   //! @param aAmount This is the number of tokens to move forward by. Default = 1.
   //! @returns The actual amount moved.
   //! @note Passing in a negative amount will not cause Input to seek backwards.
   int SeekNext(int aAmount = 1) noexcept;

   //! Moves back a specified number of tokens in the Input.
   //! Will cease moving back if at the beginning of the Input.
   //! @param aAmount This is the number of tokens to move back by. Default = 1.
   //! @returns The actual amount moved.
   //! @note Passing in a negative amount will not cause Input to seek forwards.
   int SeekPrevious(int aAmount = 1) noexcept;

   //! Searches for a specified token text, starting from the beginning of the Input. If found, goes to it.
   //! @param aString The token to search for.
   //! @returns True if the token was found. False otherwise.
   bool FindFirst(const std::string& aString);

   //! Searches for a specified token text, starting from the end of the Input. If found, goes to it.
   //! @param aString The token to search for.
   //! @returns True if the token was found. False otherwise.
   bool FindLast(const std::string& aString);

   //! Searches for a specified token text, starting from the current position and going forward. If found, goes to it.
   //! @param aString The token to search for.
   //! @returns True if the token was found. False otherwise.
   //! @note The current token IS included in the search.
   bool FindNext(const std::string& aString);

   //! Searches for a specified token text, starting from the current position and going backward. If found, goes to it.
   //! @param aString The token to search for.
   //! @returns True if the token was found. False otherwise.
   //! @note The current token IS NOT included in the search.
   bool FindPrevious(const std::string& aString);

   //! Returns the text of the current token.
   //! Returns an empty string if at the end of the input.
   std::string GetString() const;

   //! Returns the row of the current token.
   //! Returns -1 if at the end of the Input.
   int GetRow() const;

   //! Returns the column of the current token.
   //! Returns -1 if at the end of the Input.
   int GetColumn() const;

   //! Returns the text of the current token and moves Input forward by 1.
   //! Guaranteed not to walk off end of Input.
   //! Returns an empty string if at the end of the Input.
   std::string ReadString();

   //! Provides an iterator to the beginning of the Input.
   //! Lowercase for STL compatibility.
   iterator begin() const noexcept;

   //! Provides an iterator to the end of the Input.
   //! Lowercase for STL compatibility.
   iterator end() const noexcept;

   //! Provides an iterator the to current token.
   iterator ActiveIterator() const noexcept;

private:
   iterator mBegin;
   iterator mEnd;
   iterator mActive;
};

//! While a BlockInput is alive, the Input it was constructed with
//! is limited to operate within a certain range.
//! Usually limited strictly between TEXT and end_TEXT, but can be specialized
//! for other ending tokens.
class BlockInput
{
public:
   using iterator = typename Input::iterator;

   BlockInput() = delete;

   //! Sets up an Input to operate within a given range.
   //! Typically this range is strictly between TEXT and end_TEXT, where TEXT is the text of the Input's current token.
   //! @param aInput This is the Input to restrict.
   //! @throws Token if the end of the block could not be found.
   //! @see ParserLite::BlockInput::StartOfBlock
   //! @see ParserLite::BlockInput::EndOfBlock
   explicit BlockInput(Input& aInput);

   BlockInput(const BlockInput&) = delete;

   //! Allows a BlockInput to be transferred to another BlockInput.
   //! @param aOther This is the BlockInput to trasfer from.
   BlockInput(BlockInput&& aOther) noexcept;

   BlockInput& operator=(const BlockInput&) = delete;
   BlockInput& operator=(BlockInput&&) = delete; // Forces move on initialization

   //! Restores the Input's original range, and sets the Input's current token to the one immediately after the end of the block.
   virtual ~BlockInput();

protected:
   //! Finds the start of the range an Input should be restricted to.
   //! By default, this is just the current position.
   //! @param aCopyOfInput This is a copy of the Input being restricted. Seeking around this Input is safe.
   //! @param aStartOfBlock This value is set by the function to be the iterator to the start of the block.
   //! @returns True if the start can be found. False otherwise.
   virtual bool StartOfBlock(Input aCopyOfInput, iterator& aStartOfBlock) const;

   //! Finds the end of the range an Input should be restricted to.
   //! By default, this is just finds end_TEXT.
   //! @param aCopyOfInput This is a copy of the Input being restricted. Seeking around this Input is safe.
   //! @param aEndOfBlock This value is set by the function to be the iterator to the end of the block.
   //! @returns True if the end can be found. False otherwise.
   virtual bool EndOfBlock(Input aCopyOfInput, iterator& aEndOfBlock) const;

private:
   Input& mInput;

   iterator mBegin;
   iterator mEnd;
   bool     mUpdateOnDestruct;
};
} // namespace ParserLite

#endif
