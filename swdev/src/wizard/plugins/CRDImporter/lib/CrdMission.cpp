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

#include "CrdMission.hpp"

#include "CrdTransition.hpp"

namespace CrdImporter
{
CrdMission::~CrdMission() = default;

void CrdMission::SetMissionName(const std::string& aMissionName)
{
   SetName(aMissionName);
}

const std::string& CrdMission::GetMissionName() const
{
   return GetName();
}

const RouteList& CrdMission::GetRouteList() const
{
   return mRouteList;
}

void CrdMission::TakeOwnershipOfRouteList(std::vector<std::unique_ptr<Route>> aRoutes) // std::vector<std::unique_ptr<Route>>&
{
   // for (auto& route : aRoutes)
   //{
   //    mRouteList.push_back(std::move(route));
   // }
   // aRoutes.clear();
   mRouteList = std::move(aRoutes);
}

const std::string& CrdMission::GetDafifTime() const
{
   return mDafifTime;
}

void CrdMission::SetDafifTime(const std::string& aTime)
{
   mDafifTime = aTime;
}

const std::string& CrdMission::GetCreationTime() const
{
   return mCreationTime;
}

void CrdMission::SetCreationTime(const std::string& aTime)
{
   mCreationTime = aTime;
}

const PointList& CrdMission::GetPointList() const
{
   return mPointList;
}

void CrdMission::SetPointList(std::vector<std::unique_ptr<Point>>& aPoints)
{
   for (auto& point : aPoints)
   {
      mPointList[point->GetId()] = std::move(point);
   }
   aPoints.clear();
}

void CrdMission::SetVehicleList(VehicleList vehicleList)
{
   mVehicleListPtr = std::move(vehicleList);
}

void CrdMission::DeleteAllRoutes()
{
   mRouteList.clear();
}
} // namespace CrdImporter
