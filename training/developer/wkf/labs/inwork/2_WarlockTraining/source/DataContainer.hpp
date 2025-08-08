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

#ifndef WARLOCK_TRAINING_DATACONTAINER_HPP
#define WARLOCK_TRAINING_DATACONTAINER_HPP

#include <QObject>
#include <QString>

#include "Types.hpp"

namespace WarlockTraining
{
   class DataContainer : public QObject
   {
      Q_OBJECT

   public:
      PlatformData GetPlatformData(const std::string& aPlatformName) const;

      void SetData(const std::map<std::string, PlatformData>& aData);

   signals:
      void DataChanged();

   private:
      std::map<std::string, PlatformData> mData;
   };
}

#endif
