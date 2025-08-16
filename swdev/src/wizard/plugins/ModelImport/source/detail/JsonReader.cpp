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

#include <cctype>

#include "../JsonRule.hpp"

// explicit
Json::ReaderException::ReaderException(const std::string& aWhat, int aLine)
   : std::runtime_error(aWhat + " Line #" + std::to_string(aLine))
{
}

// ================================================================================================
// explicit
Json::FileReader::FileReader(std::istream& aSource)
   : mSource(aSource)
{
   Parse_();
}

// ================================================================================================
// explicit
Json::FileReader::operator bool() const noexcept
{
   return bool(mSource);
}

// ================================================================================================

bool Json::FileReader::operator!() const noexcept
{
   return !bool(*this);
}

// ================================================================================================

bool Json::FileReader::IsNull() const noexcept
{
   return mQueued == "null" && !IsString();
}

// ================================================================================================

bool Json::FileReader::IsString() const noexcept
{
   return mIsString;
}

// ================================================================================================

const std::string& Json::FileReader::Get() const noexcept
{
   return mQueued;
}

// ================================================================================================

int Json::FileReader::GetLine() const noexcept
{
   return mLineNumber;
}

// ================================================================================================
// private
void Json::FileReader::Ignore(int aAmount /*= 1*/) noexcept
{
   for (int i = 0; i < aAmount; i++)
   {
      Parse_();
   }
}

// ================================================================================================
// private
void Json::FileReader::IgnoreValue() noexcept
{
   if (Get() == "[")
   {
      ArrayReader ignoreArray(*this);
   }
   else if (Get() == "{")
   {
      ObjectReader ignoreObject(*this);
   }
   else
   {
      Ignore();
   }
}

// ================================================================================================
// private
bool Json::FileReader::CheckSymbol(const std::string& aValue, bool aPopIfTrue /*= true*/)
{
   if (Get().empty())
   {
      return false;
   }

   bool retval = Get() == aValue && !IsString();
   if (retval && aPopIfTrue)
   {
      Ignore();
   }
   return retval;
}

// ================================================================================================
// private, internal use only
void Json::FileReader::Parse_() noexcept
{
   int c = GetChar_();

   while (isspace(c))
   {
      if (c == '\n')
      {
         mLineNumber++;
      }
      c = GetChar_();
   }

   mQueued.clear();
   mIsString = false;

   // Bad read
   if (c == '\0')
   {
      return;
   }

   // Single-character tokens
   switch (c)
   {
   case '[':
   case ']':
   case '{':
   case '}':
   case ',':
   case ':':
      mQueued = c;
      return;
   }

   // Strings
   if (c == '\"')
   {
      mIsString = true;
      c         = GetChar_();
      while (c != '\"' && c != '\0')
      {
         if (c == '\\')
         {
            int escaped = GetChar_();
            switch (escaped)
            {
            case 'b':
               mQueued += '\b';
               break;
            case 'f':
               mQueued += '\f';
               break;
            case 'n':
               mQueued += '\n';
               break;
            case 'r':
               mQueued += '\r';
               break;
            case 't':
               mQueued += '\t';
               break;
            default:
               mQueued += escaped;
            }
         }
         else
         {
            mQueued += c;
         }
         c = GetChar_();
      }
      return;
   }

   // Numbers, 'true', 'false', 'null'
   if (isalnum(c) || c == '.' || c == '-' || c == '+')
   {
      mQueued += c;
      c = mSource.peek();
      while ((isalnum(c) || c == '.') && mSource)
      {
         mQueued += GetChar_();
         c = mSource.peek();
      }
   }
}

// ================================================================================================
// private, internal use only
char Json::FileReader::GetChar_() noexcept
{
   char c;
   if (!mSource.get(c))
   {
      return '\0';
   }
   return c;
}

// ================================================================================================
// explicit
Json::ArrayReader::ArrayReader(Json::FileReader& aSource)
   : mSource(aSource)
{
   if (!mSource.CheckSymbol("["))
   {
      throw ReaderException("Expected '[' for start of array.", aSource.GetLine());
   }
   if (mSource.CheckSymbol("]", false))
   {
      mEnd = true;
   }
}

// ================================================================================================

Json::ArrayReader::~ArrayReader() noexcept
{
   if (!mEnd)
   {
      int depth = 1;

      while (depth && mSource)
      {
         if (mSource.CheckSymbol("[", false))
         {
            depth++;
         }
         else if (mSource.CheckSymbol("]", false))
         {
            depth--;
         }
         mSource.Ignore(depth ? 1 : 0);
      }
   }
}

// ================================================================================================
// defined inline

/*
template<typename T>
void Json::ArrayReader::ReadValue(T& aValue)
*/

// ================================================================================================
// explicit
Json::ArrayReader::operator bool() const noexcept
{
   return !mEnd;
}

// ================================================================================================

bool Json::ArrayReader::operator!() const noexcept
{
   return !bool(*this);
}

// ================================================================================================
// explicit
Json::ObjectReader::ObjectReader(Json::FileReader& aSource)
   : mSource(aSource)
{
   if (!mSource.CheckSymbol("{"))
   {
      throw ReaderException("Expected '{' for start of object.", aSource.GetLine());
   }
   if (mSource.CheckSymbol("}", false))
   {
      mEnd = true;
   }
}

// ================================================================================================

Json::ObjectReader::~ObjectReader() noexcept
{
   if (!mEnd)
   {
      int depth = 1;

      while (depth && mSource)
      {
         if (mSource.CheckSymbol("{", false))
         {
            depth++;
         }
         else if (mSource.CheckSymbol("}", false))
         {
            depth--;
         }
         mSource.Ignore(depth ? 1 : 0);
      }
   }
}

// ================================================================================================
// defined inline

/*
template<typename Key, typename T>
void Json::ObjectReader::ReadValue(Key& aKey, T& aValue)
*/

// ================================================================================================
// defined inline

/*
template<typename Key, typename T>
bool Json::ArrayReader::ReadField(const Key& aKey, T& aValue)
*/

// ================================================================================================

void Json::ObjectReader::LoopGuard()
{
   if (!mCanReadField)
   {
      if (mEnd)
      {
         return;
      }

      if (mComma)
      {
         if (!mSource.CheckSymbol(","))
         {
            throw ReaderException("Expected ',' between Object members.", mSource.GetLine());
         }
      }
      mComma = false;

      mSource.Ignore();

      if (!mSource.CheckSymbol(":"))
      {
         throw ReaderException("Expected ':' between Object key and value.", mSource.GetLine());
      }
      mComma = true;

      mSource.IgnoreValue();

      if (mSource.CheckSymbol("}", false))
      {
         mEnd = true;
      }
   }

   mCanReadField = false;
}

// ================================================================================================
// explicit
Json::ObjectReader::operator bool() const noexcept
{
   return !mEnd;
}

// ================================================================================================

bool Json::ObjectReader::operator!() const noexcept
{
   return !bool(*this);
}

// ================================================================================================
// defined inline

/*
template<typename T>
void Json::ReadFile(const std::string& aFileName, T& aValue)
*/

// ================================================================================================
// defined inline

/*
template<typename T>
T Json::ReadFile(const std::string& aFileName)
*/
