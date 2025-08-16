// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef DATACONTAINER_HPP
#define DATACONTAINER_HPP

#include <QObject>

#include "DataTypes.hpp"


namespace WkTrackDetailsDisplay
{
class DataContainer : public QObject
{
   Q_OBJECT

public:
   void AddPlatform(const std::string& aPlatformName);
   void RemovePlatform(const std::string& aPlatformName);
   void SetData(const PlatformsTrackData& aPlatformTrackData);

   PlatformsTrackData GetData() const;

signals:
   void PlatformAdded(const std::string& aNewPlatform);
   void PlatformRemoved(const std::string& aPlatformToRemove);
   void DataUpdated();

private:
   PlatformsTrackData mData;
};
} // namespace WkTrackDetailsDisplay
#endif
