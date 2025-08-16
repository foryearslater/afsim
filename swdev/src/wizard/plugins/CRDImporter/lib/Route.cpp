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

#include "Route.hpp"

#include <cmath>

#include "CrdTransition.hpp"

namespace CrdImporter
{
Route::~Route() = default;

const PathList& Route::GetPathList() const
{
   return mPathListPtr;
}

void Route::SetPathList(PathList aPaths)
{
   mPathListPtr = std::move(aPaths);
}

const PointList& Route::GetPointList() const
{
   return mPointList;
}

void Route::AddToPointList(std::vector<std::unique_ptr<Point>>& aPoints)
{
   for (auto& point : aPoints)
   {
      mPointList[point->GetId()] = std::move(point);
   }
   aPoints.clear();
}

const RoutePointList& Route::GetRoutePointList() const
{
   return mRoutePointListPtr;
}

void Route::SetRoutePointList(RoutePointList aRoutePoints)
{
   mRoutePointListPtr = std::move(aRoutePoints);
}

const IntentList& Route::GetIntentList() const
{
   return mIntentListPtr;
}

void Route::SetIntentList(IntentList aIntents)
{
   mIntentListPtr = std::move(aIntents);
}

const StateList& Route::GetStateList() const
{
   return mStateListPtr;
}

void Route::SetStateList(StateList aStates)
{
   mStateListPtr = std::move(aStates);
}

void Route::SetVehicleReference(long aReference)
{
   mVehicleReference = aReference;
}

long Route::GetVehicleReference() const
{
   return mVehicleReference;
}
} // namespace CrdImporter
