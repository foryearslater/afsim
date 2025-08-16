// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SENSORCONTROLLERDATACONTAINER_HPP
#define SENSORCONTROLLERDATACONTAINER_HPP

#include <string>

#include <QObject>

namespace WkSensorController
{
class DataContainer : public QObject
{
   Q_OBJECT
public:
   DataContainer()                          = default;
   DataContainer(const DataContainer& aSrc) = delete;

   struct Sensor
   {
      Sensor() = default;
      Sensor(int    aId,
             bool   aCanBeTurnedOn,
             bool   aCanBeTurnedOff,
             bool   aIsSlewable,
             double aMinAz,
             double aMaxAz,
             double aMinEl,
             double aMaxEl)
         : mId(aId)
         , mCanBeTurnedOn(aCanBeTurnedOn)
         , mCanBeTurnedOff(aCanBeTurnedOff)
         , mIsSlewable(aIsSlewable)
         , mAzMin(aMinAz)
         , mAzMax(aMaxAz)
         , mElMin(aMinEl)
         , mElMax(aMaxEl)
      {
      }

      int    mId{-1};
      bool   mCanBeTurnedOn{false};
      bool   mCanBeTurnedOff{false};
      bool   mIsSlewable{false};
      double mAzMin{0};
      double mAzMax{0};
      double mElMin{0};
      double mElMax{0};
   };

   void SetData(const std::string& aPlatform,
                const std::string& aSensor,
                int                aSensorId,
                bool               aCanBeTurnedOn,
                bool               aCanBeTurnedOff,
                bool               aIsSlewable,
                double             aMinAz,
                double             aMaxAz,
                double             aMinEl,
                double             aMaxEl);

   Sensor GetData(const std::string& aPlatform, const std::string& aSensor) const;

signals:

   void SensorUpdated(const std::string& aPlatform, const std::string& aSensor);

   void SlewComplete(const std::string& aPlatform, const std::string& aSensor);

private:
   std::map<std::string, std::map<std::string, Sensor>> mPlatformToSensorMap;
};
} // namespace WkSensorController

#endif
