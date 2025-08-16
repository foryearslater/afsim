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

#include "../JsonRule.hpp"

// explicit
Json::WriterException::WriterException(const std::string& aWhat)
   : std::runtime_error(aWhat)
{
}

// ================================================================================================

Json::FileWriter::FileWriter(std::ostream& aDestination)
   : mDestination(aDestination)
{
}

// ================================================================================================
// defined inline

/*
template<typename T>
bool Json::FileWriter::Put(const T& aValue)
*/

// ================================================================================================
// defined inline

/*
template<typename T, typename... Args>
bool Json::FileWriter::Put(const T& aFirst, const Args&... aArgs)
*/

// ================================================================================================

Json::FileWriter& Json::FileWriter::PutString(const std::string& aValue)
{
   mDestination << '\"';
   for (const char c : aValue)
   {
      PutChar(c);
   }
   mDestination << '\"';
   return *this;
}

// ================================================================================================

Json::FileWriter& Json::FileWriter::PutChar(char aValue)
{
   switch (aValue)
   {
   case '\b':
      mDestination << "\\b";
      break;
   case '\f':
      mDestination << "\\f";
      break;
   case '\n':
      mDestination << "\\n";
      break;
   case '\r':
      mDestination << "\\r";
      break;
   case '\t':
      mDestination << "\\t";
      break;
   case '\"':
      mDestination << "\\\"";
      break;
   case '\\':
      mDestination << "\\\\";
      break;
   default:
      mDestination << aValue;
   }
   return *this;
}

// ================================================================================================
// private
void Json::FileWriter::NewLine()
{
   mDestination << '\n';
   for (int i = 0; i < mIndentation; i++)
   {
      mDestination << '\t';
   }
}

// ================================================================================================
// explicit
Json::ArrayWriter::ArrayWriter(Json::FileWriter& aDestination)
   : mDestination(aDestination)
{
   mDestination.mIndentation++;
   mDestination.PutRaw('[');
}

// ================================================================================================

Json::ArrayWriter::~ArrayWriter() noexcept
{
   mDestination.mIndentation--;
   if (mComma)
   {
      mDestination.NewLine();
   }
   mDestination.PutRaw(']');
}

// ================================================================================================
// defined inline

/*
template<typename T>
bool Json::ArrayWriter::WriteValue(const T& aValue)
*/

// ================================================================================================
// explicit
Json::ObjectWriter::ObjectWriter(Json::FileWriter& aDestination)
   : mDestination(aDestination)
{
   mDestination.mIndentation++;
   mDestination.PutRaw('{');
}

// ================================================================================================

Json::ObjectWriter::~ObjectWriter() noexcept
{
   mDestination.mIndentation--;
   if (mComma)
   {
      mDestination.NewLine();
   }
   mDestination.PutRaw('}');
}

// ================================================================================================
// defined inline

/*
template<typename Key, typename T>
bool Json::ObjectWriter::WriteValue(const Key& aKey, const T& aValue)
*/
