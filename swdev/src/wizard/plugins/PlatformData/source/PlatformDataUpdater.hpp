// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMDATAUPDATERS_HPP
#define PLATFORMDATAUPDATERS_HPP

#include "Environment.hpp"
#include "PlatformUpdater.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

namespace wizard
{
class Platform;
}

namespace PlatformData
{
class AltitudeUpdater : public wizard::PlatformAltitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Altitude";
   AltitudeUpdater(const std::string& aPlatformName = "");
};

class HeadingUpdater : public wizard::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Heading";
   HeadingUpdater(const std::string& aPlatformName = "");

protected:
   double GetValueInDefaultUnits() const override;
};

class LatitudeUpdater : public wizard::PlatformLatitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Latitude";
   LatitudeUpdater(const std::string& aPlatformName = "");
};

class LongitudeUpdater : public wizard::PlatformLongitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Longitude";

   LongitudeUpdater(const std::string& aPlatformName = "");
};

class PitchUpdater : public wizard::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch";

   PitchUpdater(const std::string& aPlatformName = "");
};

class RollUpdater : public wizard::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll";

   RollUpdater(const std::string& aPlatformName = "");
};

class SpeedUpdater : public wizard::PlatformSpeedUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed";

   SpeedUpdater(const std::string& aPlatformName = "");
};

class PlatformTypeUpdater : public wizard::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Type";

   PlatformTypeUpdater(const std::string& aPlatformName = "");

   QString     GetValueString() const override;
   std::string mPlatformType;
};

class PositionUpdater : public wizard::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Position";

   PositionUpdater(const std::string& aPlatformName = "");

   QString GetValueString() const override;

protected:
   double mLatitude_deg;
   double mLongitude_deg;
};
} // namespace PlatformData
#endif
