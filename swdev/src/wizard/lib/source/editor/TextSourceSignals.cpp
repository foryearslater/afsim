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

#include "TextSourceSignals.hpp"

wizard::TextSourceSignals& wizard::TextSourceSignals::GetInstance()
{
   static TextSourceSignals instance;
   return instance;
}
