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

#include "Aco.hpp"

#include <algorithm>
#include <iterator>

#include "Acmid.hpp"
#include "Efflevel.hpp"
#include "FFIRN_Validator.hpp"
#include "USMTF_Factory.hpp"
#include "ZoneFactory.hpp"

namespace usmtf
{
Aco::Aco(const std::string& aMTF, std::vector<std::unique_ptr<Set>>&& aSets) noexcept
   : Message(aMTF, std::move(aSets))
{
   BuildHeader();
   BuildZoneList();
}

void Aco::BuildHeader() noexcept
{
   const std::vector<std::string> typesForComment{"EXER", "OPER", "MSGID", "REF", "ACOID", "GEODATUM", "PERIOD", "GENTEXT", "ACMSTAT"};
   Segment::SetLocations sets = GetSets(typesForComment);
   mComments                  = ut::make_unique<Comment>(Segment{sets});
}

void Aco::BuildZoneList() noexcept
{
   const std::vector<Segment>& zoneSegments = GetSegments("ACMID");
   for (const Segment& zoneSegment : zoneSegments)
   {
      const Set&   acmid  = zoneSegment.GetSet("ACMID").first;
      const Acmid* acmidp = SetFactory::GetFactory()->CastIfRegistered<Acmid>(acmid);

      if (acmidp && acmidp->IsValid())
      {
         const std::string&    zoneType = acmidp->GetTypeOfAirspaceShape();
         std::unique_ptr<Zone> zone     = CreateZone(zoneType, zoneSegment);
         if (zone)
         {
            if (zone->IsValid())
            {
               mZones.push_back(std::move(zone));
            }
            else
            {
               // pull out zone errors for logging
               std::stringstream zoneErrorMessage{};
               zoneErrorMessage
                  << "\n**********************************************************************************\n\n";
               zoneErrorMessage << "Unable to interpret the following Segment"
                                << " as " + zoneType + " Zone.\n";
               for (const Segment::SetLocation& setLocation : zoneSegment)
               {
                  zoneErrorMessage << setLocation.first.GetReconstructedSet() << "\n";
               }
               zoneErrorMessage << "\nPlease Evaluate and Correct the following Errors: \n\n";
               AddError(zoneErrorMessage.str());
               AddErrors(zone->GetErrors());
            }
         }
         else // nullptr means zone type not supported.
         {
            // segment failed to become a zone and needs logged for end user
            AddError("Unable to interpret Segment as Zone", zoneType, "Shape is invalid zone type.");
            continue;
         }
      }
   }
   if (!IsValid()) // send out the count
   {
      std::stringstream errorSummary{};
      errorSummary << "\n**********************************************************************************\n\n";
      errorSummary << "Failed to import " << zoneSegments.size() - mZones.size() << " out of " << zoneSegments.size()
                   << " potential zones.";
      AddError(errorSummary.str());
   }
}

// This is the suggested way to reference unique pointers as a raw
// indicated YOU DO NOT OWN THIS. Open to suggestions.
std::vector<const Zone*> Aco::GetZones() const noexcept
{
   std::vector<const Zone*> zones;
   std::transform(mZones.cbegin(),
                  mZones.cend(),
                  std::back_inserter(zones),
                  [](const std::unique_ptr<Zone>& zone) -> const Zone* { return zone.get(); });

   return zones;
}

const Comment* Aco::GetHeader() const noexcept
{
   return mComments.get();
}

void ExportAco(const Aco& aAco, InputBuilder& aIb)
{
   const auto                     zones = aAco.GetZones();
   std::vector<const Exportable*> transforms{aAco.GetHeader()};
   transforms.insert(transforms.end(), zones.begin(), zones.end());
   aIb.AddBlocks(transforms);
   aIb.Export();
}
} // namespace usmtf
