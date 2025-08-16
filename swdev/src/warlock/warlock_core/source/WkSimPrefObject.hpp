// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKSIMULATIONPREFOBJECT_HPP
#define WKSIMULATIONPREFOBJECT_HPP

#include "warlock_core_export.h"

#include "WkfPrefObject.hpp"

namespace warlock
{
struct SimPrefData
{
   bool    startPaused{false};
   bool    platformsDraggable{false};
   bool    enableDIS{false};
   QString multicastIp{"228.0.0.0"};
   QString netId{"255.255.255"};
   int     port{12345};
   int     siteId{1};
   int     applicationId{1};
   int     exerciseId{1};
   bool    enabledDeferredStart{false};
   double  deferredStartTimeSec{60.0};
   double  clockRate{20.0};
};

class WARLOCK_CORE_EXPORT SimPrefObject : public wkf::PrefObjectT<SimPrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "SimPrefObject";

   SimPrefObject(QObject* parent = nullptr);

private:
   void Apply() override;

   SimPrefData ReadSettings(QSettings& aSettings) const override;
   void        SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace warlock

Q_DECLARE_METATYPE(warlock::SimPrefData)
#endif
