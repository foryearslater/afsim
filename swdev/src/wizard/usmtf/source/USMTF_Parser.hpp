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


#ifndef USMTFPARSER_HPP
#define USMTFPARSER_HPP

#include "usmtf_export.h"

#include <ios>
#include <memory>
#include <string>

#include "Message.hpp"
#include "Set.hpp"
#include "USMTF_Factory.hpp"

namespace usmtf
{
/*!
Class responsible for Parsing USMTF MIL-STD-6040, used in conjunction
with the USMTF_Factory for Generic or derived USMTF Message and Set Importing
*/
class USMTF_EXPORT USMTF_Parser
{
public:
   USMTF_Parser() = delete;
   explicit USMTF_Parser(const std::string& aFileLocation) noexcept;
   std::unique_ptr<Message> ReadMessage();

private:
   std::streampos                    AdvanceStreamToMainText(std::ifstream& aFs);
   std::vector<Field>                ReadFields(std::ifstream& aFs) noexcept;
   std::vector<std::unique_ptr<Set>> ReadSets(std::ifstream& aFs) noexcept;
   std::string                       mFileLocation;
   MessageFactory*                   mMessageFactory;
   SetFactory*                       mSetFactory;
};

} // namespace usmtf
#endif
