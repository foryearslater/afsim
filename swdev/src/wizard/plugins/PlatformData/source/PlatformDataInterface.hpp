// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMDATAINTERFACE_HPP
#define PLATFORMDATAINTERFACE_HPP

#include <set>

#include <QObject>

namespace PlatformData
{
class Interface : public QObject
{
   Q_OBJECT

public:
   Interface(QObject* parent);
   ~Interface() override = default;

   void SetPlatformOfInterest(std::string aPlatformName);

   struct PlatformData
   {
      std::string name;

      std::string type;
      std::string side;
      std::string icon;
      std::string spatial_domain;
      std::string status;
      double      startTime;
      double      endTime;

      double                                        latitude;
      double                                        longitude;
      int                                           altitude_reference;
      double                                        altitude_m;
      double                                        x;
      double                                        y;
      double                                        z;
      double                                        yaw;
      double                                        pitch;
      double                                        roll;
      double                                        psi;
      double                                        theta;
      double                                        phi;
      double                                        speed_m;
      double                                        mach;
      double                                        vel_n;
      double                                        vel_e;
      double                                        vel_d;
      double                                        vel_x;
      double                                        vel_y;
      double                                        vel_z;
      double                                        accel_m;
      double                                        accel_n;
      double                                        accel_e;
      double                                        accel_d;
      double                                        accel_x;
      double                                        accel_y;
      double                                        accel_z;
      bool                                          state_valid;
      std::set<std::string>                         categories;
      std::set<std::pair<std::string, std::string>> parts;
   };

   PlatformData GetPlatformData();

private:
   enum
   {
      LOOKUP_REQUIRED = 0,
      SCENARIO        = -1
   };
   PlatformData mPlatformData;
   int          mIndex;
};
} // namespace PlatformData
#endif // PLATFORMDATAINTERFACE_HPP
