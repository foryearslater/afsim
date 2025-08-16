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
#include "BM_SimInterface.hpp"

#include "BM_SimEvent.hpp"
#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeaponEngagement.hpp"

WkBM::SimInterface::SimInterface(const QString& aPlugin) noexcept
   : warlock::SimInterfaceT<BMSimEvent>(aPlugin)
{
}

void WkBM::SimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   auto evt = ut::make_unique<WkBM::PlatformAddedEvent>();

   evt->mIndex   = static_cast<unsigned>(aPlatform.GetIndex());
   evt->mSimTime = aSimTime;
   evt->mName    = QString::fromStdString(aPlatform.GetName());
   for (const auto& type : aPlatform.GetTypeList())
   {
      evt->mTypes += QString::fromStdString(type);
   }
   for (const auto& category : aPlatform.GetCategories().GetCategoryList())
   {
      evt->mCategories += QString::fromStdString(category);
   }
   evt->mSide     = QString::fromStdString(aPlatform.GetSide());
   evt->mIsWeapon = (WsfWeaponEngagement::Find(aPlatform) != nullptr);

   AddSimEvent(std::move(evt));
}

void WkBM::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   auto evt = ut::make_unique<WkBM::PlatformRemovedEvent>();

   evt->mIndex   = static_cast<unsigned>(aPlatform.GetIndex());
   evt->mSimTime = aSimTime;

   AddSimEvent(std::move(evt));
}

void WkBM::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   auto evt = ut::make_unique<WkBM::SimInitializingEvent>();

   std::vector<WsfPlatform*> currentTypes;
   const WsfPlatformTypes&   platformTypes = aSimulation.GetScenario().GetPlatformTypes();
   platformTypes.GetCurrentTypes(currentTypes);
   for (const auto* base : currentTypes)
   {
      evt->mTypes.append(QString::fromStdString(base->GetType()));
      auto side = QString::fromStdString(base->GetSide());
      if (!side.isEmpty() && !evt->mSides.contains(side))
      {
         evt->mSides.append(std::move(side));
      }
      for (const auto& category : base->GetCategories().GetCategoryList())
      {
         auto categoryStr = QString::fromStdString(category);
         if (!evt->mCategories.contains(categoryStr))
         {
            evt->mCategories.append(std::move(categoryStr));
         }
      }
   }

   evt->mCategories.removeDuplicates();

   AddSimEvent(std::move(evt));
   SetSimTime(0);
}

void WkBM::SimInterface::SimulationClockRead(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      SetSimTime(aSimulation.GetSimTime());
      auto size = aSimulation.GetPlatformCount();

      // Platform State Event
      {
         auto evt = ut::make_unique<WkBM::PlatformStateEvent>();
         evt->mStates.reserve(size);

         for (size_t i = 0; i < size; i++)
         {
            const auto*   platform = aSimulation.GetPlatformEntry(i);
            PlatformState ps;
            ps.mDamageFactor = platform->GetDamageFactor();
            ps.mCurrentFuel  = platform->GetFuelMass();

            evt->mStates.emplace_back(static_cast<unsigned>(platform->GetIndex()), ps);
         }

         AddSimEvent(std::move(evt));
      }

      // Aux Data Event
      {
         std::lock_guard<std::mutex> lg{mAuxDataMutex};
         if (!mAuxDataKeys.empty())
         {
            for (size_t i = 0; i < size; i++)
            {
               const auto* platform = aSimulation.GetPlatformEntry(i);
               if (platform->HasAuxData())
               {
                  auto& auxData = mAuxData[static_cast<unsigned>(platform->GetIndex())];
                  auxData.clear();
                  for (const auto& key : mAuxDataKeys)
                  {
                     try
                     {
                        UtVariant value = platform->GetAuxData().GetVar(key);
                        switch (value.GetType())
                        {
                        case UtVariant::Type::cBOOL:
                           auxData[key] = value.GetBool();
                           break;
                        case UtVariant::Type::cDOUBLE:
                           auxData[key] = value.GetDouble();
                           break;
                        case UtVariant::Type::cINT:
                           auxData[key] = value.GetInt();
                           break;
                        case UtVariant::Type::cSTRING:
                           auxData[key] = QString::fromStdString(value.GetString());
                           break;
                        case UtVariant::Type::cSTRING_ENUM:
                           auxData[key] = QString::fromStdString(value.GetStringEnum());
                           break;
                        default:
                           auxData[key].clear();
                           break;
                        }
                     }
                     catch (std::exception&)
                     {
                        // ignore
                     }
                  }
               }
            }

            auto evt      = ut::make_unique<WkBM::AuxDataEvent>();
            evt->mAuxData = mAuxData;
            AddSimEvent(std::move(evt));
         }
      }
   }
}

void WkBM::SimInterface::ClearAuxDataKeys()
{
   std::lock_guard<std::mutex> lg{mAuxDataMutex};
   mAuxDataKeys.clear();
}

void WkBM::SimInterface::AddAuxDataKey(const QString& aKey)
{
   std::string                 value = aKey.toStdString();
   std::lock_guard<std::mutex> lg{mAuxDataMutex};
   if (std::find(mAuxDataKeys.begin(), mAuxDataKeys.end(), value) == mAuxDataKeys.end())
   {
      mAuxDataKeys.emplace_back(std::move(value));
   }
}

double WkBM::SimInterface::GetSimTime() const noexcept
{
   QMutexLocker l(&mMutex);
   return mSimTime;
}

void WkBM::SimInterface::SetSimTime(double aSimTime) noexcept
{
   QMutexLocker l(&mMutex);
   mSimTime = aSimTime;
}
