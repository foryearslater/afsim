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

#include "ParserLite.hpp"

#include <iostream>

// explicit
ParserLite::FileInput::FileInput(const std::string& aPath)
{
   std::ifstream fileStream(aPath.c_str());

   if (!fileStream.is_open())
   {
      std::cout << "Could not open file: " << aPath << std::endl;
      return;
   }

   ParserLite::Token nextToken;
   int               currentRow    = 1;
   int               currentColumn = 0;

   while (GetNextToken(fileStream, nextToken, currentRow, currentColumn))
   {
      mInputStings.emplace_back(std::move(nextToken));
   }
}

// ================================================================================================

bool ParserLite::FileInput::IsEmpty() const noexcept
{
   return mInputStings.empty();
}

// ================================================================================================

ParserLite::FileInput::iterator ParserLite::FileInput::begin() const noexcept
{
   return mInputStings.begin();
}

// ================================================================================================

ParserLite::FileInput::iterator ParserLite::FileInput::end() const noexcept
{
   return mInputStings.end();
}

// ================================================================================================
// private
bool ParserLite::FileInput::GetNextToken(std::istream& aStream, ParserLite::Token& aToken, int& aRow, int& aColumn)
{
   enum class Status
   {
      Begin,
      Normal,
      ShortComment,
      LongComment,
      String
   } status = Status::Begin;

   aToken.mText.clear();

   while (aStream)
   {
      unsigned char nextChar = aStream.get();

      if (!aStream)
      {
         nextChar = '\n';
      }

      if (nextChar == '\n')
      {
         aRow++;
         aColumn = 0;
      }
      else
      {
         aColumn++;
      }

      switch (status)
      {
      case Status::Begin:
         if (nextChar == '/')
         {
            if (aStream.peek() == '/')
               status = Status::ShortComment;
            else if (aStream.peek() == '*')
            {
               aStream.ignore(); // Prevents /*/ from being a complete comment
               aColumn++;
               status = Status::LongComment;
            }
            else
            {
               aToken.mText += '/';
               aToken.mRow    = aRow;
               aToken.mColumn = aColumn;
               status         = Status::Normal;
            }
         }
         else if (nextChar == '#')
         {
            status = Status::ShortComment;
         }
         else if (nextChar == '\"')
         {
            aToken.mText += '\"';
            aToken.mRow    = aRow;
            aToken.mColumn = aColumn;
            status         = Status::String;
         }
         else if (!isspace(nextChar))
         {
            aToken.mText += nextChar;
            aToken.mRow    = aRow;
            aToken.mColumn = aColumn;
            status         = Status::Normal;
         }
         break;
      case Status::Normal:
         if (nextChar == '/')
         {
            if (aStream.peek() == '/' || aStream.peek() == '*')
            {
               aStream.putback('/');
               return true;
            }
         }
         else if (nextChar == '#' || nextChar == '\"')
         {
            aStream.putback(nextChar);
            return true;
         }
         else if (isspace(nextChar))
         {
            return true;
         }

         aToken.mText += nextChar;

         break;
      case Status::ShortComment:
         if (nextChar == '\n')
         {
            status = Status::Begin;
         }
         break;
      case Status::LongComment:
         if (nextChar == '*' && aStream.peek() == '/')
         {
            status = Status::Begin;
         }
         break;
      case Status::String:
         aToken.mText += nextChar;
         if (nextChar == '\"')
         {
            return true;
         }
         else if (nextChar == '\\')
         {
            aToken.mText += aStream.get();
            aColumn++;
         }
      }
      aStream.peek(); // Forces read past end of file, setting error flag
   }

   return false;
}

// ================================================================================================

ParserLite::Input::Input(const ParserLite::FileInput& aFile)
{
   mBegin  = aFile.begin();
   mEnd    = aFile.end();
   mActive = mBegin;
}

// ================================================================================================

bool ParserLite::Input::AtBeginning() const noexcept
{
   return mActive == mBegin;
}

// ================================================================================================

bool ParserLite::Input::AtEnd() const noexcept
{
   return mActive == mEnd;
}

// ================================================================================================

void ParserLite::Input::SeekBeginning() noexcept
{
   mActive = mBegin;
}

// ================================================================================================

void ParserLite::Input::SeekEnd() noexcept
{
   mActive = mEnd;
}

// ================================================================================================

int ParserLite::Input::SeekNext(int aAmount /*= 1*/) noexcept
{
   for (int i = 0; i < aAmount; i++)
   {
      if (AtEnd())
      {
         return i;
      }
      mActive++;
   }
   return aAmount;
}

// ================================================================================================

int ParserLite::Input::SeekPrevious(int aAmount /*= 1*/) noexcept
{
   for (int i = 0; i < aAmount; i++)
   {
      if (AtBeginning())
      {
         return i;
      }
      mActive--;
   }
   return aAmount;
}

// ================================================================================================

bool ParserLite::Input::FindFirst(const std::string& aString)
{
   for (iterator it = mBegin; it != mEnd; it++)
   {
      if (it->mText == aString)
      {
         mActive = it;
         return true;
      }
   }

   return false;
}

// ================================================================================================

bool ParserLite::Input::FindLast(const std::string& aString)
{
   iterator it = mEnd;
   it--;
   for (; it != mBegin; it--)
   {
      if (it->mText == aString)
      {
         mActive = it;
         return true;
      }
   }

   return false;
}

// ================================================================================================

bool ParserLite::Input::FindNext(const std::string& aString)
{
   for (iterator it = mActive; it != mEnd; it++)
   {
      if (it->mText == aString)
      {
         mActive = it;
         return true;
      }
   }

   return false;
}

// ================================================================================================

bool ParserLite::Input::FindPrevious(const std::string& aString)
{
   if (AtBeginning())
   {
      return false;
   }

   iterator it = mActive;
   it--;
   for (; it != mBegin; it--)
   {
      if (it->mText == aString)
      {
         mActive = it;
         return true;
      }
   }

   return false;
}

// ================================================================================================

std::string ParserLite::Input::GetString() const
{
   if (AtEnd())
   {
      return "";
   }
   return mActive->mText;
}

// ================================================================================================

int ParserLite::Input::GetRow() const
{
   if (AtEnd())
   {
      return -1;
   }
   return mActive->mRow;
}

// ================================================================================================

int ParserLite::Input::GetColumn() const
{
   if (AtEnd())
   {
      return -1;
   }
   return mActive->mColumn;
}

// ================================================================================================

std::string ParserLite::Input::ReadString()
{
   std::string returnValue = GetString();
   SeekNext();
   return returnValue;
}

// ================================================================================================

ParserLite::Input::iterator ParserLite::Input::begin() const noexcept
{
   return mBegin;
}

// ================================================================================================

ParserLite::Input::iterator ParserLite::Input::end() const noexcept
{
   return mEnd;
}

// ================================================================================================

ParserLite::Input::iterator ParserLite::Input::ActiveIterator() const noexcept
{
   return mActive;
}

// ================================================================================================
// explicit
ParserLite::BlockInput::BlockInput(ParserLite::Input& aInput)
   : mInput(aInput)
   , mBegin(aInput.begin())
   , mEnd(aInput.end())
   , mUpdateOnDestruct(false)
{
   iterator blockBegin = aInput.begin();
   iterator blockEnd   = aInput.end();

   if (StartOfBlock(aInput, blockBegin) && EndOfBlock(aInput, blockEnd))
   {
      mUpdateOnDestruct = true;
      aInput.SeekNext();
      aInput.mBegin = aInput.mActive;
      aInput.mEnd   = blockEnd;
   }
   else
   {
      throw *aInput.ActiveIterator();
   }
}

// ================================================================================================

ParserLite::BlockInput::BlockInput(ParserLite::BlockInput&& aOther) noexcept
   : mInput(aOther.mInput)
   , mBegin(aOther.mBegin)
   , mEnd(aOther.mEnd)
   , mUpdateOnDestruct(true)
{
   aOther.mUpdateOnDestruct = false;
}

// ================================================================================================
// virtual
ParserLite::BlockInput::~BlockInput()
{
   if (mUpdateOnDestruct)
   {
      mInput.mActive = mInput.mEnd;
      mInput.mBegin  = mBegin;
      mInput.mEnd    = mEnd;

      // mInput.SeekNext(); //Brings input to next token after end of block
   }
}

// ================================================================================================
// virtual
bool ParserLite::BlockInput::StartOfBlock(Input aCopyOfInput, ParserLite::BlockInput::iterator& aStartOfBlock) const
{
   aStartOfBlock = aCopyOfInput.ActiveIterator();
   return true;
}

// ================================================================================================
// virtual
bool ParserLite::BlockInput::EndOfBlock(Input aCopyOfInput, ParserLite::BlockInput::iterator& aEndOfBlock) const
{
   if (aCopyOfInput.FindNext(std::string("end_") + aCopyOfInput.GetString()))
   {
      aEndOfBlock = aCopyOfInput.ActiveIterator();
      return true;
   }
   return false;
}
