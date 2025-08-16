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

#include "Ato.hpp"

#include "AircraftMission.hpp"
#include "AircraftMissionTransform.hpp"
#include "FFIRN_Validator.hpp"
#include "USMTF_Factory.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"

namespace usmtf
{
Ato::Ato(const std::string& aMTF, std::vector<std::unique_ptr<Set>>&& aSets) noexcept
   : Message(aMTF, std::move(aSets))
{
   BuildHeader();
   BuildPlatformList();
}

void Ato::BuildHeader() noexcept
{
   const std::vector<std::string> typesForComment{"EXER", "OPER", "MSGID", "AKNLDG", "TIMEFRAM", "TSKCNTRY", "SVCTASK", "AMPN"};
   Segment::SetLocations          sets = GetSets(typesForComment);
   mComments                           = ut::make_unique<Comment>(Segment{sets});
}
// This is the suggested way to reference unique pointers as a raw
// indicated YOU DO NOT OWN THIS. Open to suggestions.
std::vector<const Platform*> Ato::GetPlatforms() const noexcept
{
   std::vector<const Platform*> platforms;
   std::transform(mPlatforms.cbegin(),
                  mPlatforms.cend(),
                  std::back_inserter(platforms),
                  [](const std::unique_ptr<Platform>& platform) -> const Platform* { return platform.get(); });

   return platforms;
}

const Comment* Ato::GetHeader() const noexcept
{
   return mComments.get();
}

void Ato::BuildPlatformList() noexcept
{
   std::vector<Segment> taskUnitSegments = GetSegments("TASKUNIT");
   for (const Segment& segment : taskUnitSegments)
   {
      ProcessTaskUnit(segment);
   }
}

void Ato::ProcessTaskUnit(const Segment& taskUnitSegment)
{
   const Segment::SetLocation& taskUnit = taskUnitSegment.GetSet("TASKUNIT"); // has to be task unit or we wouldnt be here.
   std::vector<Segment::SetLocations> aircraftMissionSegments = taskUnitSegment.GetSubSegments("MSNACFT");
   for (Segment::SetLocations aircraftMissionSegment : aircraftMissionSegments)
   {
      aircraftMissionSegment.push_back(taskUnit);
      if (taskUnitSegment.HasSet("AMSNDAT"))
      {
         aircraftMissionSegment.push_back(taskUnitSegment.GetSet("AMSNDAT"));
      }

      AircraftMissionTransform amt{Segment{aircraftMissionSegment}};
      // Bubble up errors
      if (amt.IsValid())
      {
         for (const Platform& platform : amt.GetPlatforms())
         {
            // Get Macros out
            AddMacros(amt.GetMacros());
            // Get Platforms out
            mPlatforms.push_back(ut::make_unique<Platform>(platform));
         }
      }
      else
      {
         std::stringstream errorMessage{};
         errorMessage << "\n**********************************************************************************\n\n";
         errorMessage << "Unable to Process Segment:\n";
         for (const Segment::SetLocation& setLocation : aircraftMissionSegment)
         {
            errorMessage << setLocation.first.GetReconstructedSet() << "\n";
         }
         errorMessage << "\nRemember, at this time only segments that contain MSNACFT and AMSNDAT are supported.\n";
         errorMessage << "Please Evaluate and Correct the following Errors: \n\n";
         AddError(errorMessage.str());
         AddErrors(amt.GetErrors());
      }
   }
}

void ExportAto(const Ato& aAto, const std::string& aDirectory, const std::string& aFileName)
{
   UtPath typeDefinitionsDirectory{aDirectory + "/type_definitions"};
   if (!typeDefinitionsDirectory.Exists())
   {
      typeDefinitionsDirectory.Mkdir();
   }

   InputBuilder                               laydown{aDirectory + "/" + aFileName};
   std::vector<const PlatformTypeDefinition*> types{};
   std::vector<const Exportable*>             commentsSection{aAto.GetHeader()};
   laydown.AddBlocks(commentsSection);


   const auto platforms = aAto.GetPlatforms();

   std::transform(platforms.cbegin(),
                  platforms.cend(),
                  std::back_inserter(types),
                  [](const Platform* platform) -> const PlatformTypeDefinition*
                  { return &platform->GetTypeDefinition(); });

   std::vector<std::string> alreadyIncluded{};
   InputBlock               include{"", "", true, true};
   for (const PlatformTypeDefinition* definition : types)
   {
      std::string definitionName      = definition->GetDefinition();
      std::string definitionFileName  = definitionName + ".txt";
      UtPath      typeDefinitionsFile = typeDefinitionsDirectory + definitionFileName;
      if (!typeDefinitionsFile.Exists())
      {
         InputBuilder typesBuilder{aDirectory + "/type_definitions/" + definitionFileName};
         typesBuilder.AddBlock(definition);
         typesBuilder.Export();
      }

      if (std::find(alreadyIncluded.begin(), alreadyIncluded.end(), definitionName) == alreadyIncluded.end())
      {
         include.AddProperty({"include_once", "./type_definitions/" + definitionFileName, " "});
         alreadyIncluded.push_back(definitionName);
      }
   }
   laydown.AddBlock(include);

   // Just putting them into a single block for spacing.
   const auto         macros = aAto.GetMacros();
   InputBlock         macroElaboration{"", "", true, true};
   InputBlockProperty comment{"#Macro Definitions", "", ""};
   InputBlockProperty
      comment2{"#These are placeholder variables for the platforms that did not include an actionable position.", "", ""};
   InputBlockProperty comment3{"#You must fill these out with supported AFSIM position information.", "", ""};
   macroElaboration.AddProperty(comment);
   macroElaboration.AddProperty(comment2);
   macroElaboration.AddProperty(comment3);
   laydown.AddBlock(macroElaboration);
   InputBlock macroList{"", "", true, true};
   for (const Macro& macro : macros)
   {
      InputBlock                             ib    = macro.GetInputBlock();
      const std::vector<InputBlockProperty>& props = ib.GetProperties();
      for (InputBlockProperty prop : props)
      {
         macroList.AddProperty(prop);
      }
   }
   laydown.AddBlock(macroList);
   const std::vector<const Exportable*> platformBlocks{platforms.begin(), platforms.end()};
   laydown.AddBlocks(platformBlocks);
   laydown.Export();
}
} // namespace usmtf
