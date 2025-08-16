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

#include "AircraftMissionTransform.hpp"

#include "Platform.hpp"

namespace usmtf
{
AircraftMissionTransform::AircraftMissionTransform(const Segment& aSegment) noexcept
   : USMTF_Transformer(aSegment)
{
   // task unit can provide categorical grouping for created platforms.
   // platform definition information.
   mTaskUnit = ExtractSet<TaskUnit>("TASKUNIT", "Aircraft Mission"); // also can contain position, this position, if
                                                                     // present, takes precedent.
   mMission = ExtractSet<AircraftMission>("MSNACFT", "Aircraft Mission"); // platform definition, type, macro and number
                                                                          // of them. If no position set is provided, macro
   mAerialRefueling =
      ExtractSet<AerialRefueling>("5REFUEL", "Aircraft Mission", false, true); // AerialRefueling only, these are the
                                                                               // platforms receiving fuel
   mPackageData =
      ExtractSet<PackageData>("9PKGDAT", "Aircraft Mission", false, true); // number, unit, Callsign and type, one of the
                                                                           // entries should match msnacft, the others
   // are added
   // other possibilities for type information
   // RESASSET

   // Unit position information
   mMissionData = ExtractSet<AircraftMissionData>("AMSNDAT", "Aircraft Mission"); // recovery points of interest? Departure
                                                                                  // location! still optional...
   mAerialRefuelingInfo =
      ExtractSet<AerialRefuelingInfo>("ARINFO", "Aircraft Mission", false, true); // position, however its optional.
   mAirAssetcontrol =
      ExtractSet<AirAssetControl>("CONTROLA", "Aircraft Mission", false, true); // report in point may be used.
   // o ther possibilities for Unit position information not selected for beta
   //  RESASSET location is still optional
   //  7Control report in point is optional


   // Target location information.
   mMissionLocaton =
      ExtractSet<AircraftMissionLocation>("ASMNLOC", "Aircraft Mission", false, true); // position, however its optional.
   // other possible not selected for beta
   // GTGTLOC
   // SHIPTGT
   // AIRMOVE
   // ESCDATA
   // RECCEDAT
   // PTRCPLOT
   // MTGTLOC


   // does not seem useful or I have forgotten their purpose.
   // mRefuelTasking = ExtractSet<RefuelingTasking>("REFTSK", "Aircraft Mission", false, true);
   // mPackageCommander = ExtractSet<PackageCommander>("PKGCMD", "Aircraft Mission", false, true); doesnt seem useful.
   BuildPlatforms();
}

void AircraftMissionTransform::BuildPlatforms()
{
   if (!IsValid())
   {
      return;
   }
   if (mMission && mMission->IsValid())
   {
      for (int i = 0; i < mMission->GetNumberOfAircraft(); i++)
      {
         std::string            definition = ReplaceSpaces(mMission->GetAircraftType(), '_');
         PlatformTypeDefinition typeDefinition{definition, "WSF_PLATFORM"};
         Platform               platform{};
         platform.SetTypeDefinition(typeDefinition);
         platform.SetName(ReplaceSpaces(mMission->GetAircraftCallsign(), '_') + "_" + std::to_string(i));
         platform.SetCategory(ReplaceSpaces(mTaskUnit->GetUnitDesignator().GetText(), '_'));
         SetPriorityPosition(platform);
         mPlatforms.push_back(std::move(platform));
      }
      if (mAerialRefueling && mAerialRefueling->IsValid())
      {
         for (const AerialRefueling::RefuelInstruction& instruction : mAerialRefueling->GetRefuelInstructions())
         {
            FreeText               nameOfPlatform    = instruction.mRecieverCallsign;
            int                    numberOfPlatforms = instruction.mAllocatedAircraft;
            FreeText               typeOfPlatform    = instruction.mTypeOfAircraft;
            std::string            definition        = ReplaceSpaces(typeOfPlatform.GetText(), '_');
            PlatformTypeDefinition typeDefinition{definition, "WSF_PLATFORM"};
            for (int i = 0; i < numberOfPlatforms; i++)
            {
               Platform platform{};
               platform.SetTypeDefinition(typeDefinition);
               platform.SetName(ReplaceSpaces(nameOfPlatform.GetText() + "_REFUEL", '_') + "_" + std::to_string(i));
               platform.SetCategory(ReplaceSpaces(mTaskUnit->GetUnitDesignator().GetText(), '_'));
               SetPriorityPosition(platform);
               mPlatforms.push_back(std::move(platform));
            }
         }
      }
      if (mPackageData && mPackageData->IsValid())
      {
         for (const PackageData::Unit& instruction : mPackageData->GetUnits())
         {
            FreeText               unit              = std::get<1>(instruction);
            int                    numberOfPlatforms = std::get<4>(instruction);
            FreeText               typeOfPlatform    = std::get<5>(instruction);
            FreeText               callsign          = std::get<6>(instruction);
            std::string            definition        = ReplaceSpaces(typeOfPlatform.GetText(), '_');
            PlatformTypeDefinition typeDefinition{definition, "WSF_PLATFORM"};
            for (int i = 0; i < numberOfPlatforms; i++)
            {
               Platform platform{};
               platform.SetTypeDefinition(typeDefinition);
               platform.SetName(ReplaceSpaces(callsign.GetText() + "_PACKAGEDATA", '_') + "_" + std::to_string(i));
               platform.SetCategory(ReplaceSpaces(unit.GetText(), '_'));
               SetPriorityPosition(platform);
               mPlatforms.push_back(std::move(platform));
            }
         }
      }
   }
}

void AircraftMissionTransform::SetPriorityPosition(Platform& platform)
{
   if (mTaskUnit && mTaskUnit->IsValid() && !mTaskUnit->GetLocation().GetLocation().empty())
   {
      const NamedLocation& location = mTaskUnit->GetLocation();
      if (location.IsLocationFreeForm())
      {
         platform.SetPosition(UseMacro(location.GetLocation()));
      }
      else
      {
         platform.SetPosition(location.GetLocation());
      }
   }
   else if (mMissionData && mMissionData->IsValid() && mMissionData->GetDepartureLocation().HasMatch())
   {
      if (UTM_Location* location = mMissionData->GetDepartureLocation().IsMatch<UTM_Location>())
      {
         platform.SetPosition(location->GetFormattedLocation());
      }
      else if (NamedLocation* location = mMissionData->GetDepartureLocation().IsMatch<NamedLocation>())
      {
         if (location->IsLocationFreeForm())
         {
            platform.SetPosition(UseMacro(location->GetLocation()));
         }
         else
         {
            platform.SetPosition(location->GetLocation());
         }
      }
      else if (FreeText* location = mMissionData->GetDepartureLocation().IsMatch<FreeText>())
      {
         platform.SetPosition(UseMacro(location->GetText()));
      }
      else
      {
         platform.SetPosition(UseMacro(mTaskUnit->GetUnitDesignator().GetText()));
      }
      // should we consider recovery locations as viable?
      // VariantValidator<> departureLoc = mMissionData->GetRecoveryLocations();
   }
   else if (mAerialRefuelingInfo && mAerialRefuelingInfo->IsValid() && mAerialRefuelingInfo->GetControlPoint().HasMatch())
   {
      const VariantValidator<LatLon, RelativeBearingRange, FreeText>& controlPoint =
         mAerialRefuelingInfo->GetControlPoint();
      if (FreeText* cp = controlPoint.IsMatch<FreeText>())
      {
         platform.SetPosition(UseMacro(cp->GetText()));
      }
      else if (LatLon* cp = controlPoint.IsMatch<LatLon>())
      {
         platform.SetPosition(cp->GetFormattedLatLon());
      }
      else
      {
         platform.SetPosition(UseMacro(mTaskUnit->GetUnitDesignator().GetText()));
      }
   }
   else if (mAirAssetcontrol && mAirAssetcontrol->IsValid() && mAirAssetcontrol->GetReportInPoint().HasMatch())
   {
      if (UTM_Location* location = mAirAssetcontrol->GetReportInPoint().IsMatch<UTM_Location>())
      {
         platform.SetPosition(location->GetFormattedLocation());
      }
      else if (LatLon* location = mAirAssetcontrol->GetReportInPoint().IsMatch<LatLon>())
      {
         platform.SetPosition(location->GetFormattedLatLon());
      }
      else if (VerifiedLatLonTM* location = mAirAssetcontrol->GetReportInPoint().IsMatch<VerifiedLatLonTM>())
      {
         platform.SetPosition(location->GetFormattedLatLon());
      }
      else if (VerifiedLatLonDS* location = mAirAssetcontrol->GetReportInPoint().IsMatch<VerifiedLatLonDS>())
      {
         platform.SetPosition(location->GetFormattedLatLon());
      }
      else if (FreeText* location = mAirAssetcontrol->GetReportInPoint().IsMatch<FreeText>())
      {
         platform.SetPosition(UseMacro(location->GetText()));
      }
      else
      {
         platform.SetPosition(UseMacro(mTaskUnit->GetUnitDesignator().GetText()));
      }
   }
   else
   {
      // task unit name Macro.
      platform.SetPosition(UseMacro(mTaskUnit->GetUnitDesignator().GetText()));
   }
}

const std::string AircraftMissionTransform::UseMacro(const std::string& aMacroName)
{
   Macro positionMacro{ReplaceSpaces(aMacroName, '_'), " <latitude-value> <longitude-value>"};
   AddMacro(positionMacro);
   return positionMacro.GetPlaceholder();
}

InputBlock AircraftMissionTransform::GetInputBlock() const
{
   // Use GetPlatforms instead if youd like more control and flexibility over export positioning.
   InputBlock root{"", "", true, true};
   for (const Platform& p : mPlatforms)
   {
      root.AddBlock(p.GetTypeDefinition().GetInputBlock());
      root.AddBlock(p.GetInputBlock());
   }
   return root;
}

const std::vector<Platform>& AircraftMissionTransform::GetPlatforms() const noexcept
{
   return mPlatforms;
}
} // namespace usmtf
