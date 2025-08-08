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

#ifndef WARLOCK_TRAINING_TYPES_HPP
#define WARLOCK_TRAINING_TYPES_HPP

namespace WarlockTraining
{
   struct PlatformData
   {
      double mLatitude = 0;
      double mLongitude = 0;
      double mAltitude = 0;
      double mHeading = 0;
   };
}

#endif