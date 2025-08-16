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

#ifndef CYBERENGAGEMENTCONTROLLERDATACONTAINER_HPP
#define CYBERENGAGEMENTCONTROLLERDATACONTAINER_HPP

#include <string>
#include <vector>

#include <QObject>

#include "WkfEnvironment.hpp"
#include "WsfCyberAttackTypes.hpp"

namespace WkCyberEngagementController
{
class DataContainer : public QObject
{
   Q_OBJECT
public:
   DataContainer() = default;

   void                            SetTypes(std::vector<std::string>& aAttackTypes) { mAttackTypes = aAttackTypes; }
   const std::vector<std::string>& GetTypes() const { return mAttackTypes; }

private:
   std::vector<std::string> mAttackTypes;
};
} // namespace WkCyberEngagementController

#endif
