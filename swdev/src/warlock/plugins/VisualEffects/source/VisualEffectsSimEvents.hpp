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

#ifndef VISUALEFFECTSSIMEVENTS_HPP
#define VISUALEFFECTSSIMEVENTS_HPP

#include <memory>
#include <string>

#include <osg/Vec4>

#include "UtMemory.hpp"
#include "VaEntity.hpp"
#include "VaEnvironment.hpp"
#include "VaModelDatabase.hpp"
#include "VaPosition.hpp"
#include "VaTimeManager.hpp"
#include "VaViewerManager.hpp"
#include "WkSimInterface.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "visual_effect/WkfVisualEffects.hpp"
#include "visual_effect/WkfVisualEffectsDisplayInterface.hpp"

namespace WkVisualEffects
{
struct EffectInfo
{
   vespa::VaPosition mPosition;
   std::string       mSourcePlatformName;
};
} // namespace WkVisualEffects

class VisualEffectsSimEvent : public warlock::SimEvent
{
public:
   virtual void Process(wkf::VisualEffectsDisplayInterface* aDisplay) = 0;
};

template<class EFFECT_TYPE>
class EffectEvent : public VisualEffectsSimEvent
{
public:
   EffectEvent(const WkVisualEffects::EffectInfo& aInfo, const std::string& aDescription)
      : mInfo(aInfo)
      , mDescription(aDescription)
   {
      static_assert(
         std::is_base_of<wkf::VisualEffect, EFFECT_TYPE>::value,
         "Template argument to EffectEvent in VisualEffectsSimEvents must be a subclass of wkf::VisualEffect!");
   }

   void Process(wkf::VisualEffectsDisplayInterface* aDisplay) override
   {
      // std::is_base_of is a compile-time check, not a runtime one. To get around this,
      // two different ProcessEffect functions exist: one for TrailingEffects and one for
      // static VisualEffects.
      ProcessEffect(aDisplay, std::is_base_of<wkf::TrailingEffect, EFFECT_TYPE>{});
   }

private:
   // Returns a vector of available engine data for the given platform.
   // Prioritizes finding the engine coordinates for the model used in immersive views,
   // then checks the model used in the Map Display.
   std::vector<osg::Vec4> GetEngineData(wkf::Platform& aPlatform)
   {
      const vespa::VaModelDefinition* modelDef = vaEnv.GetModelDatabase()->GetModelDefinition(aPlatform.GetIcon());
      if (modelDef != nullptr)
      {
         for (const auto& view : vaEnv.GetViewerManager()->GetViewers())
         {
            UtVariant   v    = view->GetData("type");
            std::string type = v.GetString();
            if (type == "immersive")
            {
               return modelDef->GetEngines(vaEnv.GetModelDatabase()->GetViewerModelSet(*view));
            }
         }

         if (vaEnv.GetStandardViewer() != nullptr)
         {
            return modelDef->GetEngines(vaEnv.GetModelDatabase()->GetViewerModelSet(*vaEnv.GetStandardViewer()));
         }
      }

      return std::vector<osg::Vec4>();
   }

   // Handles TrailingEffects.
   void ProcessEffect(wkf::VisualEffectsDisplayInterface* aDisplay, std::true_type)
   {
      wkf::Scenario* scenario = vaEnv.GetStandardScenario();
      if (scenario != nullptr)
      {
         wkf::Platform* platform = scenario->FindPlatformByName(mInfo.mSourcePlatformName);
         if (platform != nullptr)
         {
            // First, determine if this effect emanates from an engine. If it does, we need to create
            // an effect for each engine the source platform has.
            auto effect = ut::make_unique<EFFECT_TYPE>();
            if (effect->IsFromEngine())
            {
               std::vector<osg::Vec4> engineData = GetEngineData(*platform);
               if (engineData.empty())
               {
                  engineData.emplace_back(osg::Vec4(0, 0, 0, -1));
               }

               for (const auto& engine : engineData)
               {
                  effect = ut::make_unique<EFFECT_TYPE>();
                  effect->Initialize();
                  effect->SetCreationTime(vaEnv.GetTime()->GetWallClockTime());
                  effect->SetOffset(osg::Vec3(engine.x(), engine.y(), engine.z()));
                  effect->SetSourcePlatform(*platform);

                  if (engine.w() >= 0)
                  {
                     effect->SetInitialParticleSize(engine.w());
                  }

                  if (effect->IsShort())
                  {
                     aDisplay->AddEffect(*platform, std::move(effect));
                  }
                  else
                  {
                     wkf::Scenario* scenario = vaEnv.GetStandardScenario();
                     if (scenario != nullptr)
                     {
                        auto entity = new vespa::VaEntity(mDescription);
                        entity->SetPosition(mInfo.mPosition);
                        if (scenario->AddEntity(entity))
                        {
                           aDisplay->AddEffect(*entity, std::move(effect));
                        }
                        else
                        {
                           delete entity;
                        }
                     }
                  }
               }
            }
            else
            {
               effect->Initialize();
               effect->SetCreationTime(vaEnv.GetTime()->GetWallClockTime());
               effect->SetSourcePlatform(*platform);
               if (effect->IsShort())
               {
                  aDisplay->AddEffect(*platform, std::move(effect));
               }
               else
               {
                  wkf::Scenario* scenario = vaEnv.GetStandardScenario();
                  if (scenario != nullptr)
                  {
                     auto entity = new vespa::VaEntity(mDescription);
                     entity->SetPosition(mInfo.mPosition);
                     if (scenario->AddEntity(entity))
                     {
                        aDisplay->AddEffect(*entity, std::move(effect));
                     }
                     else
                     {
                        delete entity;
                     }
                  }
               }
            }
         }
      }
   }

   // Handles static VisualEffects.
   void ProcessEffect(wkf::VisualEffectsDisplayInterface* aDisplay, std::false_type)
   {
      auto effect = ut::make_unique<EFFECT_TYPE>();
      effect->Initialize();
      effect->SetCreationTime(vaEnv.GetTime()->GetWallClockTime());

      wkf::Scenario* scenario = vaEnv.GetStandardScenario();
      if (scenario != nullptr)
      {
         auto entity = new vespa::VaEntity(mDescription);
         entity->SetPosition(mInfo.mPosition);
         if (scenario->AddEntity(entity))
         {
            aDisplay->AddEffect(*entity, std::move(effect));
         }
         else
         {
            delete entity;
         }
      }
   }

   WkVisualEffects::EffectInfo mInfo;
   std::string                 mDescription;
};

class EffectPauseOrResumeEvent : public VisualEffectsSimEvent
{
public:
   EffectPauseOrResumeEvent(bool aPause);

   void Process(wkf::VisualEffectsDisplayInterface* aDisplayPtr) override;

private:
   bool mPause;
};

template<class EFFECT_TYPE>
class RemoveTrailingEffectEvent : public VisualEffectsSimEvent
{
public:
   RemoveTrailingEffectEvent(const std::string& aPlatformName, bool aGradual)
      : mSourcePlatformName(aPlatformName)
      , mGradual(aGradual)
   {
      static_assert(std::is_base_of<wkf::TrailingEffect, EFFECT_TYPE>::value,
                    "Template argument to RemoveTrailingEffectEvent in VisualEffectsSimEvents must be a subclass of "
                    "wkf::TrailingEffect!");
   }

   void Process(wkf::VisualEffectsDisplayInterface* aDisplayPtr) override
   {
      wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
      if (scenarioPtr != nullptr)
      {
         wkf::Platform* platform = scenarioPtr->FindPlatformByName(mSourcePlatformName);
         if (platform != nullptr)
         {
            aDisplayPtr->RemoveEffectsFromPlatform<EFFECT_TYPE>(mGradual, *platform);
         }
      }
   }

private:
   std::string mSourcePlatformName;
   bool        mGradual;
};

#endif
