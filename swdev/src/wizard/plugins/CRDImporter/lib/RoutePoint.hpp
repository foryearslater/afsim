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

#ifndef ROUTEPOINT_HPP
#define ROUTEPOINT_HPP

#include "ElementBase.hpp"

namespace CrdImporter
{
class RoutePoint : public ElementBase
{
public:
   RoutePoint()           = default;
   ~RoutePoint() override = default;

   long GetReferencePointId() const { return mPointReference; }
   void SetReferencePointId(long aId) { mPointReference = aId; }

private:
   long mPointReference{0};
};
} // namespace CrdImporter
#endif
