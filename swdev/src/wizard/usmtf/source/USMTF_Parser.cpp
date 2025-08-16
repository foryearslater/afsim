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

#include "USMTF_Parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "Field.hpp"
#include "Message.hpp"
#include "Set.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"

namespace
{
void Ltrim(std::string& aVal) noexcept
{
   aVal.erase(aVal.begin(), std::find_if(aVal.begin(), aVal.end(), [](int ch) { return !std::isspace(ch); }));
}

void Rtrim(std::string& aVal) noexcept
{
   aVal.erase(std::find_if(aVal.rbegin(), aVal.rend(), [](int ch) { return !std::isspace(ch); }).base(), aVal.end());
}

void StripWhiteSpace(std::string& aFieldValue) noexcept
{
   Ltrim(aFieldValue);
   Rtrim(aFieldValue);
}
} // namespace

namespace usmtf
{
const static char cFIELD_MARKER = '/'; // Delimiter that indicates the start of a Field.

// A USMTF Message starts its main text, i.e. the first governed sections, at
// one of these two set ids. Parser uses these delimiters to seek past introduction text.
const static std::string cMAIN_TEXT_START_EXER = "EXER";
const static std::string cMAIN_TEXT_START_OPER = "OPER";
const static std::string cMESSAGE_ID_SET_TYPE  = "MSGID";

// Used to move the file stream in the parser back to the first set that identifies the main text.
const static size_t      cBYTE_OFFSET     = cMAIN_TEXT_START_EXER.length() + 1;
const static std::string cREVISION_NUMBER = "2004";
const static std::string cUSMTF_STANDARD  = "MIL-STD-6040";

USMTF_Parser::USMTF_Parser(const std::string& aFileLocation) noexcept
   : mFileLocation(std::move(aFileLocation))
   , mMessageFactory(MessageFactory::GetFactory())
   , mSetFactory(SetFactory::GetFactory())
{
}

std::unique_ptr<Message> USMTF_Parser::ReadMessage()
{
   std::ifstream fs(mFileLocation);
   if (!fs)
   {
      throw ImportError("Failed to open file: " + mFileLocation);
   }

   AdvanceStreamToMainText(fs);
   std::vector<std::unique_ptr<Set>> sets = ReadSets(fs);

   if (sets.size() < 2)
   {
      throw ImportError("Invalid USMTF Message as defined by " + cUSMTF_STANDARD + " Revision " + cREVISION_NUMBER +
                        ": Message must contain at least 2 Sets, EXER/OPER and MSGID.");
   }
   // Message ID should always be set at position 2 after EXPER or OPER
   const Set& messageIdentifierSet = *sets[1];

   if (messageIdentifierSet.GetType() != cMESSAGE_ID_SET_TYPE)
   {
      throw ImportError("Invalid USMTF Message as defined by " + cUSMTF_STANDARD + " Revision " + cREVISION_NUMBER +
                        ": Message must contain MSGID Set in Position 2 to identify Message Type");
   }
   const Field&       field       = messageIdentifierSet.GetField(1);
   const std::string& messageType = field.GetContent();

   return mMessageFactory->CreateInstance(messageType, std::move(sets));
}

std::streampos USMTF_Parser::AdvanceStreamToMainText(std::ifstream& fs)
{
   std::string    currentWord;
   std::streampos position;
   while (std::getline(fs, currentWord, cFIELD_MARKER))
   {
      StripWhiteSpace(currentWord);
      if (currentWord == cMAIN_TEXT_START_EXER || currentWord == cMAIN_TEXT_START_OPER)
      {
         // reset stream to point before the word is found. This is the start of the main text and first set
         position = static_cast<int>(fs.tellg()) - cBYTE_OFFSET;
         fs.seekg(position);
         return position;
      }
   }
   throw ImportError("Invalid USMTF Message as defined by " + cUSMTF_STANDARD + " Revision " + cREVISION_NUMBER +
                     ": Message must contain either a EXER or OPER set that starts main text.");
}

std::vector<Field> USMTF_Parser::ReadFields(std::ifstream& aFs) noexcept
{
   std::vector<Field> fields;
   std::string        fieldValue;
   while (std::getline(aFs, fieldValue, cFIELD_MARKER))
   {
      if (!fieldValue.empty())
      {
         StripWhiteSpace(fieldValue);
         fields.emplace_back(fieldValue);
      }
      else
      {
         return fields;
      } // end of set
   }
   return fields; // no fields.
}

std::vector<std::unique_ptr<Set>> USMTF_Parser::ReadSets(std::ifstream& aFs) noexcept
{
   std::vector<std::unique_ptr<Set>> sets;

   // use file stream to read sets until done. each set construction goes
   // through factory, if set type is known it constructs it, if not the base Set is instantiated.
   // This allows read of USMTF file without taking it entirely into memory. I am not sure how large these files can be
   // as its not indicated in the specification.
   while (aFs)
   {
      std::vector<Field> fields = ReadFields(aFs);
      if (fields.empty())
      {
         // Skip sets with no fields as they aren't valid, bonus solve whitespace bug.
         continue;
      }
      std::string stf = fields[0].GetContent();
      if (stf.empty()) // end of file, don't add this set, it will be empty
      {
         break;
      }

      std::unique_ptr<Set> set = mSetFactory->CreateInstance(stf, std::move(fields));
      sets.push_back(std::move(set));
   }
   return sets;
}
} // namespace usmtf
