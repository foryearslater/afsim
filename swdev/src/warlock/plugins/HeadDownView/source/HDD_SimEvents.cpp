// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "HDD_SimEvents.hpp"

void HDD::UpdateHDDDataEvent::Process(DataContainer& aState)
{
   aState.SetHDDData(mData.GetHDDData());
}
