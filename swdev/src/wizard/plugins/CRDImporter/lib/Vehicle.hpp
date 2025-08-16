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

#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <map>
#include <memory>

#include "ElementBase.hpp"

namespace CrdImporter
{
class Vehicle : public ElementBase
{
public:
   Vehicle() { SetId(-1); }
   ~Vehicle() override = default;
};
using VehicleList = std::unique_ptr<std::map<long, std::unique_ptr<Vehicle>>>;
} // namespace CrdImporter
#endif
