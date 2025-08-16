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
#ifndef WKPLUGIN_HPP
#define WKPLUGIN_HPP

#include "UtMemory.hpp"
#include "WkPlotUpdater.hpp"
#include "WkSimInterface.hpp"
#include "WkfPlugin.hpp"

namespace warlock
{
// A class template representing a Warlock plugin that interacts with a WSF simulation.
// Most plugins will derive from the class template warlock::PluginT below.
template<class PClass>
class PluginBaseT : public PClass
{
   static_assert(std::is_base_of<wkf::Plugin, PClass>::value,
                 "Template argument to warlock::PluginBaseT must be a subclass of wkf::Plugin");

public:
   virtual std::unique_ptr<warlock::PlotUpdater> GetPlotUpdater(const QString& aPlatformName,
                                                                const QString& aY,
                                                                const unsigned aContext = 0) const
   {
      return nullptr;
   }

protected:
   using Plugin = PluginBaseT<PClass>;
   using Roles  = typename PClass::Roles;

   PluginBaseT(const QString& aPluginName, const size_t aUniqueId, const QFlags<Roles>& aRoles = PClass::eNO_ROLE)
      : PClass(aPluginName, aUniqueId, aRoles)
   {
   }

   ~PluginBaseT() override = 0;
};

template<class PClass>
PluginBaseT<PClass>::~PluginBaseT() = default;

using Plugin = PluginBaseT<wkf::Plugin>;

// A class template that extends warlock::PluginBaseT by instantiating a thread-safe warlock::SimInterface to interact
// directly with the simulation object. Template argument must be a subclass of warlock::SimInterface
template<class IClass, class PClass = wkf::Plugin>
class PluginT : public PluginBaseT<PClass>
{
   static_assert(std::is_base_of<SimInterfaceBase, IClass>::value,
                 "Template argument to warlock::PluginT must be a subclass of warlock::SimInterfaceT");

   using BaseType = PluginBaseT<PClass>;
   using Roles    = typename BaseType::Roles;

protected:
   PluginT(const QString& aPluginName, const size_t aUniqueId, const QFlags<Roles>& aRoles = PClass::eNO_ROLE)
      : BaseType(aPluginName, aUniqueId, aRoles)
      , mInterfacePtr(ut::make_unique<IClass>(aPluginName))
   {
   }

   ~PluginT() override = 0;

   IClass* GetInterface() const { return mInterfacePtr.get(); }

   std::unique_ptr<IClass> mInterfacePtr;
};

template<class IClass, class PClass>
PluginT<IClass, PClass>::~PluginT() = default;
} // namespace warlock
#endif // WKPLUGIN_HPP
