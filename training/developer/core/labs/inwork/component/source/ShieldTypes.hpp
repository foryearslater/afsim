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

#ifndef SHIELDTYPES_HPP
#define SHIELDTYPES_HPP

#include "ShieldComponent.hpp"
#include "WsfObjectTypeList.hpp"

class ShieldTypes : public WsfObjectTypeList<ShieldComponent>
{
   public:
      //! @name Static methods to return a reference to the type list associated with a scenario.
      //@{
      static ShieldTypes& Get(WsfScenario& aScenario);
      static const ShieldTypes& Get(const WsfScenario& aScenario);
      //@}

      explicit ShieldTypes(WsfScenario& aScenario);
};

#endif
