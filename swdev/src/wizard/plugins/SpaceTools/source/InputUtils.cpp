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

#include "InputUtils.hpp"

#include <tuple>

#include "ProxyWatcher.hpp"
#include "WsfPM_Root.hpp"

namespace // {anonymous}
{

void ReplaceWithSpace(std::string& aText, size_t aStartPos, size_t aEndPos)
{
   size_t count = aEndPos - aStartPos;
   aText.replace(aStartPos, count, count, ' ');
}

} // namespace

//! Return the range in the given text containing the next comment.
//!
//! This returns the range of @p aText that contains the next comment starting from the
//! position @p aStartPos. If one of the two components of the returned std::pair is
//! equal to std::string::npos, then either no comment was found, or there were not well-formed
//! comments in the given @p aText. The first component of the returned std::pair will
//! indicate the position in @p aText just after the start-of-comment marker ('#', '//' or '/*').
//! The second component of the returned std::pair will indicate the position in @p aText
//! just before the end-of-comment marker ('\n', '\n' or '*/, respectively).
//!
//! \param aText     - The text being considered.
//! \param aStartPos - The offset into @p aText from which to begin the search.
//! \returns         - A std::pair containing the start and end offsets of a found comment.
//!                    One of the components will be std::string::npos if no well-formed comment was found.
std::pair<size_t, size_t> SpaceTools::InputUtils::FindNextComment(const std::string& aText, size_t aStartPos)
{
   static std::string cPOUND_START{"#"};
   static std::string cNEWLINE_END{"\n"};
   static std::string cSLASHES_START{"//"};
   static std::string cCSTYLE_START{"/*"};
   static std::string cCSTYLE_END{"*/"};


   size_t startC_Style = aText.find(cCSTYLE_START, aStartPos);
   size_t startSlashes = aText.find(cSLASHES_START, aStartPos);
   size_t startPound   = aText.find(cPOUND_START, aStartPos);
   size_t start        = std::min<size_t>({startC_Style, startSlashes, startPound});
   size_t end{};
   if (start != std::string::npos)
   {
      if (start == startC_Style)
      {
         start += cCSTYLE_START.size();
         end = aText.find(cCSTYLE_END, start);
      }
      else if (start == startPound)
      {
         start += cPOUND_START.size();
         end = aText.find(cNEWLINE_END, start);
      }
      else if (start == startSlashes)
      {
         start += cSLASHES_START.size();
         end = aText.find(cNEWLINE_END, start);
      }
   }

   return std::make_pair(start, end);
}

//! Handle any comments in the section of the input stack under consideration.
//!
//! This method protects against injecting the generated mission_sequence block into
//! regions of the input stack that are commented out. This method replaces the
//! contents of the comment (excluding the comment markers, '#', '//', or '/*' and '*/')
//! with spaces. This method replaces the comments with spaces instead of removing them
//! because the return value is used to compute a range of offsets in the originating
//! document that will be replaced by the generated input command. If the comments are
//! not replaced character for character, that output will end up in the wrong place in the
//! input stack. The reason this operation is performed is to avoid false detections of
//! mission_sequence, mover or end_platform commands that may have been commented out.
//!
//! \param aText - The text to have comments replaced with whitespace.
//! \returns     - @p aText but with comment contents replaced with whitespace.
std::string SpaceTools::InputUtils::HandleComments(const std::string& aText)
{
   std::string retval{aText};

   size_t currentPos{0};
   while (currentPos != std::string::npos)
   {
      size_t startPos;
      size_t endPos;
      std::tie(startPos, endPos) = FindNextComment(aText, currentPos);
      if (startPos != std::string::npos && endPos != std::string::npos)
      {
         ReplaceWithSpace(retval, startPos, endPos);
         currentPos = endPos;
      }
      else
      {
         // No comment found, all done
         currentPos = startPos;
      }
   }

   return retval;
}

std::vector<std::pair<UtTextDocumentRange, size_t>> SpaceTools::InputUtils::GetBlockRanges(const std::string& aBlock,
                                                                                           const std::string& aPlatformName)
{
   std::string blockClose{std::string{"end"} + aBlock};

   std::vector<std::pair<UtTextDocumentRange, size_t>> retval{};

   WsfPM_Platform      platformNode = GetPlatformProxy(aPlatformName);
   UtTextDocumentRange range{};
   if (!platformNode.IsNull())
   {
      UtTextDocumentRange sourceRange = platformNode.GetSourceDocumentRange();
      std::string         sourceText  = InputUtils::HandleComments(sourceRange.Text());

      size_t startPos = sourceText.find(aBlock);
      if (startPos != std::string::npos)
      {
         size_t endPos = sourceText.find(blockClose, startPos);
         if (endPos != std::string::npos)
         {
            size_t line{};
            size_t column{0};
            sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
            startPos -= column;
            range = UtTextDocumentRange{sourceRange.mSource,
                                        sourceRange.GetBegin() + startPos,
                                        sourceRange.GetBegin() + endPos + blockClose.size() - 1};

            retval.emplace_back(range, column);

            size_t nextStart = sourceText.find(aBlock, endPos + blockClose.size() - 1);
            while (nextStart != std::string::npos)
            {
               size_t nextEnd = sourceText.find(blockClose, nextStart);
               if (nextEnd != std::string::npos)
               {
                  sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + nextStart, line, column);
                  UtTextDocumentRange extraRange{sourceRange.mSource,
                                                 sourceRange.GetBegin() + nextStart - column,
                                                 sourceRange.GetBegin() + nextEnd + blockClose.size() - 1};
                  retval.emplace_back(extraRange, column);
               }
               nextStart = sourceText.find(aBlock, nextEnd + blockClose.size() - 1);
            }
         }
      }
   }

   return retval;
}

WsfPM_Platform SpaceTools::InputUtils::GetPlatformProxy(const std::string& aPlatformName)
{
   WsfPProxy*        proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root        rootNode{proxyPtr};
   WsfPM_PlatformMap map = rootNode.platforms();
   return map.FindT(aPlatformName);
}
