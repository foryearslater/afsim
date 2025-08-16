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
#ifndef CRDMISSION_HPP
#define CRDMISSION_HPP

#include <map>
#include <memory>

#include "ElementBase.hpp"
#include "Route.hpp"
#include "Vehicle.hpp"

namespace CrdImporter
{
class CrdMission : public ElementBase
{
public:
   CrdMission() = default;
   ~CrdMission() override;

   const std::string& GetMissionName() const;
   void               SetMissionName(const std::string& aMissionName);

   const RouteList& GetRouteList() const;
   // could change the vector to be unique ptr to vector and eliminate the loop in the definition - just move the unique ptr
   void TakeOwnershipOfRouteList(std::vector<std::unique_ptr<Route>> aRoutes);

   const std::string& GetDafifTime() const;
   void               SetDafifTime(const std::string& aTime);

   const std::string& GetCreationTime() const;
   void               SetCreationTime(const std::string& aTime);

   const PointList& GetPointList() const;
   // could change the vector to be unique ptr to vector and eliminate the loop in the definition - just move the unique ptr
   void SetPointList(std::vector<std::unique_ptr<Point>>& aPoints);

   void SetVehicleList(VehicleList vehicleList);
   void DeleteAllRoutes();

private:
   RouteList   mRouteList;
   PointList   mPointList;
   VehicleList mVehicleListPtr;
   std::string mDafifTime;
   std::string mCreationTime;
};
} // namespace CrdImporter
#endif
