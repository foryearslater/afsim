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

#include "DisEntityId.hpp"
#include "DisEntityType.hpp"
#include "WkPlatformUpdater.hpp"

namespace WkPlatformData
{
class AltitudeUpdater : public warlock::PlatformAltitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Altitude";

   AltitudeUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformAltitudeUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class HeadingUpdater : public warlock::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Heading";

   HeadingUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class LatitudeUpdater : public warlock::PlatformLatitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Latitude";

   LatitudeUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformLatitudeUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueDouble() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class LongitudeUpdater : public warlock::PlatformLongitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Longitude";

   LongitudeUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformLongitudeUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueDouble() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class PitchUpdater : public warlock::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch";

   PitchUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class RollUpdater : public warlock::PlatformAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll";

   RollUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class SpeedUpdater : public warlock::PlatformSpeedUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed";

   SpeedUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformSpeedUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class MachUpdater : public warlock::PlatformUnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Mach";

   MachUpdater(const std::string& aPlatformName = "");

protected:
   double GetValueDouble() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class DamageFactorUpdater : public warlock::PlatformUnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Damage Factor";

   DamageFactorUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueDouble() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class FuelUpdater : public warlock::PlatformMassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel";

   FuelUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformMassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueDouble() const override;
   bool   ReadData(WsfPlatform* aPlatform) override;
};

class PositionUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Position";

   PositionUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mLatitude_deg(0.0)
      , mLongitude_deg(0.0)
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   double mLatitude_deg;
   double mLongitude_deg;
};

class PlatformTypeUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Type";

   PlatformTypeUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mPlatformType()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   std::string mPlatformType;
};

class PlatformSideUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Side";

   PlatformSideUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mPlatformSide()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   std::string mPlatformSide;
};

class PlatformDomainUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Spatial Domain";

   PlatformDomainUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mPlatformDomain()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   WsfSpatialDomain mPlatformDomain;
};

class PlatformIndexUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Index";

   PlatformIndexUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mIndex(0)
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   size_t mIndex;
};

class PlatformLocalityUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Locality";

   PlatformLocalityUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mLocality()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   QString mLocality;
};

class PlatformDIS_IdUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "DIS Id";

   PlatformDIS_IdUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mDIS_Id()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   DisEntityId mDIS_Id;
};

class PlatformDIS_TypeUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "DIS Type";

   PlatformDIS_TypeUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mDIS_Type()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   DisEntityType mDIS_Type;
};

class PlatformDIS_MarkingUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Marking";

   PlatformDIS_MarkingUpdater(const std::string& aPlatformName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mDIS_Marking()
   {
   }

   QString GetValueString() const override;

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   QString mDIS_Marking;
};

class PlatformAuxDataUpdater : public warlock::PlatformUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "AuxData";

   PlatformAuxDataUpdater(const std::string& aPlatformName = "", const std::string& aDatumName = "")
      : warlock::PlatformUpdater(cUPDATER_NAME, aPlatformName)
      , mDatumName(aDatumName)
      , mValue()
   {
   }

   QString       GetValueString() const override;
   const QString GetDisplayName() const override { return mDatumName.c_str(); }

protected:
   bool ReadData(WsfPlatform* aPlatform) override;

   std::string mDatumName;
   QString     mValue;
};
} // namespace WkPlatformData
#endif
