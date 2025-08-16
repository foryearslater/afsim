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

#ifndef INPUTWRITER_HPP
#define INPUTWRITER_HPP

#include <string>
#include <utility>

#include "UtTextDocument.hpp"
#include "WsfPM_Platform.hpp"
class WsfScenario;

namespace wizard
{

class Project;

}

namespace wsfg
{
namespace spaceg
{

class OrbitalMissionModel;

} // namespace spaceg
} // namespace wsfg

namespace SpaceTools
{

//! This object handles writing mission sequences into the input stack.
class InputWriter
{
public:
   explicit InputWriter(wizard::Project& aProject);

   void ProduceOutput(const std::string& aPlatformName, const wsfg::spaceg::OrbitalMissionModel* aModelPtr);

private:
   void                  SetupSequenceRange(const std::string& aPlatformName);
   bool                  SetupIndent(const std::string& aLineText, std::size_t aColumn);
   static WsfPM_Platform GetPlatformProxy(const std::string& aPlatformName);
   std::string           GetReplacementText(const wsfg::spaceg::OrbitalMissionModel* aModelPtr) const;
   void                  ReplaceText(const std::string& aReplacementText);

   wizard::Project&                 mProject;
   UtTextDocumentRange              mRange{};
   std::vector<UtTextDocumentRange> mErasures{};
   int                              mIndentLevel{0};
   int                              mIndentSize{0};
   bool                             mTrailingNewline{false};
   bool                             mWrapInEditMoverBlock{false};
   bool                             mAddTrailingEndMover{false};
   bool                             mAddTrailingEndPlatform{false};
   bool                             mAddInitialNewline{false};
};

} // namespace SpaceTools

#endif // INPUTWRITER_HPP
