// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "InputWriter.hpp"

#include <cctype>
#include <sstream>

#include "EditorPrefObject.hpp"
#include "InputUtils.hpp"
#include "OrbitalMissionModel.hpp"
#include "OrbitalModelToSequence.hpp"
#include "OrbitalSequenceToInput.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "SpaceToolsScenario.hpp"
#include "TextSource.hpp"
#include "UtCast.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Root.hpp"

SpaceTools::InputWriter::InputWriter(wizard::Project& aProject)
   : mProject(aProject)
{
}

//! Write the given mission sequence into the input stack for the given platform.
//!
//! This method writes the given @p aModelPtr into the input stack for the platform
//! with the given @p aPlatformName. If there is some difficulty finding an
//! appropriate place to inject the mission sequence commands, this will raise a
//! dialog box with a message indicating the problem, and no output will be performed.
//!
//! \param aPlatformName - The name of the platform of interest.
//! \param aModelPtr     - The mission model to output.
void SpaceTools::InputWriter::ProduceOutput(const std::string&                       aPlatformName,
                                            const wsfg::spaceg::OrbitalMissionModel* aModelPtr)
{
   SetupSequenceRange(aPlatformName);
   if (mRange.Valid())
   {
      mProject.OpenFile(mRange.mSource->GetFilePath().GetSystemPath().c_str());
      std::string replacementText = GetReplacementText(aModelPtr);
      ReplaceText(replacementText);
   }
}

//! Find the range in the input stack into which output will be produced.
//!
//! This method sets up this object by finding the relevant range in the appropriate
//! document for adding commands for a mission sequence for the platform with the given
//! @p aPlatformName. There are three possibilities for where the input is injected into
//! the input stack:
//!   1) Overwriting an existing mission_sequence block,
//!   2) At the end of a mover block,
//!   3) At the end of the platform block.
//! These possibilities are considered in the given order.
void SpaceTools::InputWriter::SetupSequenceRange(const std::string& aPlatformName)
{
   static std::string cBLOCK_OPEN{"mission_sequence"};
   static std::string cBLOCK_CLOSE{"end_mission_sequence"};
   static std::string cFALLBACK_MARKER{"end_mover"};
   static std::string cFINAL_FALLBACK{"end_platform"};

   mIndentSize = ut::cast_to_int(wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace());

   WsfPM_Platform platNode = GetPlatformProxy(aPlatformName);
   if (!platNode.IsNull())
   {
      UtTextDocumentRange sourceRange = platNode.GetSourceDocumentRange();
      std::string         sourceText  = InputUtils::HandleComments(sourceRange.Text());

      // First look for a mission_sequence block.
      size_t startPos = sourceText.find(cBLOCK_OPEN);
      if (startPos != std::string::npos)
      {
         size_t endPos = sourceText.find(cBLOCK_CLOSE, startPos);
         if (endPos != std::string::npos)
         {
            size_t line{};
            size_t column{};
            sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
            mIndentLevel     = ut::cast_to_int(column) / mIndentSize;
            mTrailingNewline = false;
            startPos -= column;
            mRange = UtTextDocumentRange{sourceRange.mSource,
                                         sourceRange.GetBegin() + startPos,
                                         sourceRange.GetBegin() + endPos + cBLOCK_CLOSE.size() - 1};

            // Look for other mission_sequence blocks that will be removed on write.
            size_t nextStart = sourceText.find(cBLOCK_OPEN, endPos + cBLOCK_CLOSE.size() - 1);
            while (nextStart != std::string::npos)
            {
               size_t nextEnd = sourceText.find(cBLOCK_CLOSE, nextStart);
               if (nextEnd != std::string::npos)
               {
                  sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + nextStart, line, column);
                  mErasures.emplace_back(sourceRange.mSource,
                                         sourceRange.GetBegin() + nextStart - column,
                                         sourceRange.GetBegin() + nextEnd + cBLOCK_CLOSE.size());
               }
               nextStart = sourceText.find(cBLOCK_OPEN, nextEnd + cBLOCK_CLOSE.size() - 1);
            }
         }
         else
         {
            std::stringstream ss{};
            ss << "Error in '" << sourceRange.mSource->GetFilePath().GetSystemPath()
               << "'. Unable to locate end of mission sequence. No modifications will be made.";
            QMessageBox::warning(nullptr, QString("Astrolabe Error"), QString::fromStdString(ss.str()), QMessageBox::Ok);
         }
      }
      else
      {
         // If there is no mission_sequence, then look for a mover block, and add the mission_sequence
         // at the end of that block.
         startPos = sourceText.find(cFALLBACK_MARKER);
         if (startPos != std::string::npos)
         {
            size_t line{};
            size_t column{};
            sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
            size_t lineStart     = startPos - column;
            mTrailingNewline     = true;
            mAddTrailingEndMover = true;

            // Setup the indent level, and detect if the found token was the first in the line.
            bool wasFirstToken = SetupIndent(sourceRange.mSource->GetLineString(line), column);

            if (wasFirstToken)
            {
               ++mIndentLevel;
               mRange = UtTextDocumentRange{sourceRange.mSource,
                                            sourceRange.GetBegin() + lineStart,
                                            sourceRange.GetBegin() + startPos + cFALLBACK_MARKER.size() - 1};
            }
            else
            {
               mAddInitialNewline = true;
               mRange             = UtTextDocumentRange{sourceRange.mSource,
                                            sourceRange.GetBegin() + startPos,
                                            sourceRange.GetBegin() + startPos + cFALLBACK_MARKER.size() - 1};
            }
         }
         else
         {
            // If there is also no mover block found, then attempt to place it at the end of the
            // platform block.
            startPos = sourceText.find(cFINAL_FALLBACK);
            if (startPos != std::string::npos)
            {
               size_t line{};
               size_t column{};
               sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
               size_t lineStart        = startPos - column;
               mTrailingNewline        = true;
               mWrapInEditMoverBlock   = true;
               mAddTrailingEndPlatform = true;

               // Setup the indent level, and detect if the found token was the first in the line.
               bool wasFirstToken = SetupIndent(sourceRange.mSource->GetLineString(line), column);
               mIndentLevel += 2;

               if (wasFirstToken)
               {
                  mRange = UtTextDocumentRange{sourceRange.mSource,
                                               sourceRange.GetBegin() + lineStart,
                                               sourceRange.GetBegin() + startPos + cFINAL_FALLBACK.size() - 1};
               }
               else
               {
                  mAddInitialNewline = true;
                  mRange             = UtTextDocumentRange{sourceRange.mSource,
                                               sourceRange.GetBegin() + startPos,
                                               sourceRange.GetBegin() + startPos + cFINAL_FALLBACK.size() - 1};
               }
            }
            else
            {
               std::stringstream ss{};
               ss << "Error in '" << sourceRange.mSource->GetFilePath().GetSystemPath()
                  << "'. Unable to locate appropriate place to insert mission sequence. No modifications will be made.";
               QMessageBox::warning(nullptr, QString("Astrolabe Error"), QString::fromStdString(ss.str()), QMessageBox::Ok);
            }
         }
      }
   }
   else
   {
      QMessageBox::warning(nullptr,
                           QString("Astrolabe Error"),
                           QString("Unable to locate '") + QString::fromStdString(aPlatformName) +
                              QString("' in the input stack. No modifications to the input will be made."),
                           QMessageBox::Ok);
   }
}

bool SpaceTools::InputWriter::SetupIndent(const std::string& aLineText, std::size_t aColumn)
{
   size_t firstToken{0};
   for (; firstToken < aColumn && firstToken < aLineText.size(); ++firstToken)
   {
      if (!std::isspace(aLineText[firstToken]))
      {
         break;
      }
   }
   mIndentLevel = ut::cast_to_int(firstToken) / mIndentSize;
   return firstToken == aColumn;
}

WsfPM_Platform SpaceTools::InputWriter::GetPlatformProxy(const std::string& aPlatformName)
{
   WsfPProxy*        proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root        rootNode{proxyPtr};
   WsfPM_PlatformMap map = rootNode.platforms();
   return map.FindT(aPlatformName);
}

std::string SpaceTools::InputWriter::GetReplacementText(const wsfg::spaceg::OrbitalMissionModel* aModelPtr) const
{
   // Output anything needed before the mission sequence.
   std::stringstream ss{};
   if (mAddInitialNewline)
   {
      ss << "\n";
   }
   if (mWrapInEditMoverBlock)
   {
      int indentArgument = std::max(0, mIndentLevel - 1);
      ss << std::string(indentArgument * mIndentSize, ' ') << "edit mover\n";
   }

   // Output the mission sequence.
   Scenario                                   scenario{};
   wsfg::spaceg::OrbitalModelToSequence       modelToSequence{scenario.GetScenario()};
   std::unique_ptr<WsfOrbitalMissionSequence> sequencePtr = modelToSequence.Transform(aModelPtr->GetMissionData());
   OrbitalSequenceToInput                     sequenceToInput{ss, mIndentLevel, mIndentSize, mTrailingNewline};
   sequencePtr->Accept(&sequenceToInput);

   // Output anything needed after the mission sequence.
   if (mWrapInEditMoverBlock || mAddTrailingEndMover)
   {
      int indentArgument = std::max(0, mIndentLevel - 1);
      ss << std::string(indentArgument * mIndentSize, ' ') << "end_mover";
   }
   if (mAddTrailingEndPlatform)
   {
      int indentArgument = std::max(0, mIndentLevel - 2);
      ss << "\n" << std::string(indentArgument * mIndentSize, ' ') << "end_platform";
   }

   return ss.str();
}

//! Replace the text with the generated sequence.
//!
//! Note that this also removes extra mission sequence blocks.
void SpaceTools::InputWriter::ReplaceText(const std::string& aReplacementText)
{
   std::string platformSourcePath = mRange.mSource->GetFilePath().GetSystemPath();
   wizard::ProjectWorkspace::Instance()->GotoRange(mRange);
   wizard::TextSource* platformSourcePtr =
      wizard::ProjectWorkspace::Instance()->FindSource(platformSourcePath, false, true);

   // Must remove the additional ranges before writing the new.
   // Further, we must remove them in reverse order for the ranges to be sensible.
   for (auto iter = mErasures.rbegin(); iter != mErasures.rend(); ++iter)
   {
      platformSourcePtr->Replace(*iter, "");
   }

   if (mRange.Text() != aReplacementText)
   {
      platformSourcePtr->Replace(mRange, aReplacementText);
   }
}
