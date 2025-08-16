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

#ifndef CRDROUTE_HPP
#define CRDROUTE_HPP

#include <map>
#include <memory>
#include <vector>

#include "CrdState.hpp"
#include "ElementBase.hpp"
#include "Intent.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "RoutePoint.hpp"

namespace CrdImporter
{
using IntentList     = std::unique_ptr<std::map<long, std::unique_ptr<Intent>>>;
using PathList       = std::unique_ptr<std::map<long, std::unique_ptr<Path>>>;
using PointList      = std::map<long, std::unique_ptr<Point>>;
using RoutePointList = std::unique_ptr<std::map<long, std::unique_ptr<RoutePoint>>>;
using StateList      = std::unique_ptr<std::map<long, std::unique_ptr<CrdState>>>;

class Route : public ElementBase
{
public:
   Route() = default;
   ~Route() override;

   Route(const Route&) = delete;
   Route& operator=(const Route&) = delete;

   const PathList& GetPathList() const;
   void            SetPathList(PathList aPaths);

   const PointList& GetPointList() const;
   void             AddToPointList(std::vector<std::unique_ptr<Point>>& aPoints);

   const RoutePointList& GetRoutePointList() const;
   void                  SetRoutePointList(RoutePointList aRoutePoints);

   const IntentList& GetIntentList() const;
   void              SetIntentList(IntentList aIntents);

   const StateList& GetStateList() const;
   void             SetStateList(StateList aStates);

   void SetVehicleReference(long aReference);
   long GetVehicleReference() const;

private:
   PointList      mPointList;
   IntentList     mIntentListPtr;
   PathList       mPathListPtr;
   RoutePointList mRoutePointListPtr;
   StateList      mStateListPtr;
   long           mVehicleReference;
};

using RouteList = std::vector<std::unique_ptr<Route>>; // std::map<long, std::unique_ptr<Route>>;
} // namespace CrdImporter
#endif
