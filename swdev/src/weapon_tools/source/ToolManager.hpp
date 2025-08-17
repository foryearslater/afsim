// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TOOLMANAGER_HPP
#define TOOLMANAGER_HPP

#include <vector>

#include "Tool.hpp"
#include "WeaponToolsExport.hpp"
class UtInput;
#include "WsfObjectTypeList.hpp"
#include "WsfScenarioExtension.hpp"

//! ToolManager contains a repository for all existing tool types, and will inform a simulation
//! when all registered Tools have completed their work.  Each derived instance of the Tool class
//! will automatically register via CheckIn() to the class singleton to add itself into the
//! collection of known tools that are instantiated.  Once an instance is added, it must then also
//! CheckOut() from this object when it is complete.  When each of the tools that have registered
//! then CheckOut(), the simulation is fully complete, and may terminate.

class WT_EXPORT ToolManager : public WsfScenarioExtension
{
public:
   static ToolManager* Find(WsfScenario& aScenario);
   ToolManager(WsfScenario& aScenario);
   void AddedToScenario() override;
   ~ToolManager() override;

   bool ProcessInput(UtInput& aInput) override;
   bool Initialize(WsfSimulation& aSimulation);

   bool AllToolsDone();

   //! This informs the ToolManager that a tool instance is finished.
   bool UnRegister(Tool* aToolPtr);

   //! This informs the ToolManager that a tool instance is now active (in use).
   bool Register(Tool* aToolPtr);

   bool AddToolType(const std::string& aType, Tool* aToolTypePtr);

private:
   void AddBaseTypes();

   typedef std::vector<std::pair<Tool*, bool>> ToolsStatus;

   ToolsStatus             mRunningToolsStatus;
   WsfObjectTypeList<Tool> mTypeList;
};

#endif
