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

#ifndef WKXIO_DATACONTAINER_HPP
#define WKXIO_DATACONTAINER_HPP

class WsfPlatform;
class WsfXIO_Connection;
class WsfXIO_PlatformInfo;

#include "warlock_core_export.h"

#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_RemotePlatformList.hpp"
#include "xio/WsfXIO_SerializeTypes.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT XIO_DataContainer
{
public:
   XIO_DataContainer() = default;

   WsfXIO_PlatformInfo* GetPlatformInfo(const WsfPlatform* aPlatform) const;
   WsfXIO_PlatformInfo* GetPlatformInfo(const WsfXIO_Connection* aConnectionPtr, const unsigned int aHostPlatformIndex) const;

   WsfXIO_RemotePlatformList* GetPlatformList() const { return mXIO_PlatformListPtr.get(); }

   void Clear();

   void Initialize(WsfXIO_Interface* aXIO_Interface);

   bool AddConnection(WsfXIO_Connection* aConnectionPtr, const WsfClockSource* aClockPtr);
   void DropConnection(WsfXIO_Connection* aConnectionPtr);

private:
   std::unique_ptr<WsfXIO_RemotePlatformList> mXIO_PlatformListPtr;
};
} // namespace warlock
#endif