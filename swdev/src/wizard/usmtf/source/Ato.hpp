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

#ifndef ATO_HPP
#define ATO_HPP

#include "usmtf_export.h"

#include <memory>

#include "Comment.hpp"
#include "InputBuilder.hpp"
#include "MacroEnabled.hpp"
#include "Message.hpp"
#include "Platform.hpp"

namespace usmtf
{
/*!
   PURPOSE: THE ATO IS USED TO TASK AIR MISSIONS, ASSIGN CROSS-FORCE TASKING, AND INTRASERVICE TASKING.
   GENERAL INSTRUCTIONS: THE ORIGINAL ATO WILL BE GIVEN A UNIQUE IDENTIFICATION IN FIELD 3 OF THE MSGID
   SET FOR THE SPECIFIED TIME FRAME (TIMEFRAM). CHANGES TO THE ORIGINAL ATO WILL NORMALLY BE ISSUED WITH A
   SUBSEQUENT ATO MESSAGE IDENTIFIED AS A CHANGE (CHG) IN FIELD 5 OF THE MSGID SET. FURTHER CHANGES WILL
   BE SEQUENTIALLY NUMBERED.
*/

/* Programmer Commentary:
   For our purposes the AIR TASKING ORDER (ATO) Message describes various entities used for AFSIM Scenario lay down.
   Primarily its used to extract platforms, as thats the most concrete entity. A zone is present for area of interest as
   well. Many of the data elements in an ATO do not fulfill all the requirements of its AFSIM entity counterpart, this
   means that the user will have to fill in the missing data that is needed. This means that the ATO import primarily
   serves to build a scaffolding for the user to fill in that missing data easily with the use of macros.

   For examples see tests/message_examples/Ato.txt
   Detailed Document:
   ATO.UF.html
*/
class USMTF_EXPORT Ato : public Message, public MacroEnabled
{
public:
   Ato(const std::string& aMTF, std::vector<std::unique_ptr<Set>>&& aSets) noexcept;
   //! Comment section for useful data that will be places at the top of the export.
   const Comment* GetHeader() const noexcept;
   //! Platforms extracted from Message
   std::vector<const Platform*> GetPlatforms() const noexcept;

private:
   //! Extracts Platforms from the transform for later use in exportation.
   void BuildPlatformList() noexcept;
   void BuildHeader() noexcept;
   //! Takes a TaskUnit Segment Block and uses the transform to get platforms
   //! and or extract errors from the validation process.
   void                                   ProcessTaskUnit(const Segment& taskUnitSegment);
   std::unique_ptr<Comment>               mComments;
   std::vector<std::unique_ptr<Platform>> mPlatforms;
};

//! Exports the Ato to scenario lay down file indicated by the target InputBuilder
USMTF_EXPORT void ExportAto(const Ato& aAto, const std::string& aDirectory, const std::string& aFileName);
} // namespace usmtf

#endif
