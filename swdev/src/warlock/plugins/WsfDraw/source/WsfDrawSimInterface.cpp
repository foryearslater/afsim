// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WsfDrawSimInterface.hpp"

#include "UtMemory.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WsfDraw.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSimulation.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"
#include "xio/WsfXIO_DrawRequest.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

WkWsfDraw::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<WsfDrawEvent>(aPluginName)
   , mReceiveDraws(false)
{
}

void WkWsfDraw::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   mCallbacks.Clear();

   AddSimEvent(ut::make_unique<SimInitializingEvent>());

   WsfDrawManager* drawManager = WsfDrawManager::Find(aSimulation);
   if (drawManager != nullptr)
   {
      mCallbacks.Add(drawManager->DrawUpdate.Connect(
         [this, drawManager](const std::string& aTarget, const std::string& aLayer, const std::vector<WsfDraw::DrawEvent>& aEvents)
         { AddSimEvent(ut::make_unique<DrawEvent>(drawManager->GetSimulation().GetSimTime(), aLayer, aEvents)); }));
   }

   WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
   if (xio)
   {
      mCallbacks += xio->Connect(&WkWsfDraw::SimInterface::HandleDrawPkt, this);

      mCallbacks += xio->OnConnected.Connect(
         [=](WsfXIO_Connection* aConnectionPtr)
         {
            if (aConnectionPtr->IsReliable())
            {
               xio->GetRequestManager().AddRequest(new WsfXIO_DrawRequest(aConnectionPtr));
            }
         });
   }

   mSimulation = &aSimulation;
}

void WkWsfDraw::SimInterface::HandleDrawPkt(WsfXIO_DrawPkt& aPkt)
{
   if (!mReceiveDraws)
   {
      return;
   }
   std::string                     layer;
   std::vector<WsfDraw::DrawEvent> events;
   auto*                           senderPtr = dynamic_cast<WsfXIO_Connection*>(aPkt.GetSender());
   for (auto& entity : aPkt.mEntities)
   {
      if (layer.empty())
      {
         layer = aPkt.mDictionary.Decode(entity.mLayerName);
      }

      WsfDraw::DrawEvent event;

      event.mIcon = aPkt.mDictionary.Decode(entity.mIconName);
      event.mText = aPkt.mDictionary.Decode(entity.mText);

      event.mDuration    = entity.mDuration;
      event.mLineSize    = entity.mLineSize;
      event.mPointSize   = entity.mPointSize;
      event.mTextSize    = entity.mTextSize;
      event.mLineStyle   = entity.mLineStyle;
      event.mHeading     = entity.mHeading;
      event.mPitch       = entity.mPitch;
      event.mRoll        = entity.mRoll;
      event.mEllipseMode = entity.mEllipseMode;
      event.mAxisA       = entity.mAxisA;
      event.mAxisB       = entity.mAxisB;
      event.mAxisC       = entity.mAxisC;

      event.mDrawType = WsfDraw::DrawType(entity.mDrawType);
      event.mID       = entity.mId;

      float red    = static_cast<float>(entity.mColor[0]) / 255;
      float green  = static_cast<float>(entity.mColor[1]) / 255;
      float blue   = static_cast<float>(entity.mColor[2]) / 255;
      float alpha  = static_cast<float>(entity.mColor[3]) / 255;
      event.mColor = UtColor(red, green, blue, alpha);

      event.mVerts[0].mVertexType = WsfDraw::VertexType(entity.mVerts[0].mVertexType);
      event.mVerts[0].mPosition   = entity.mVerts[0].mPosition;
      event.mVerts[1].mVertexType = WsfDraw::VertexType(entity.mVerts[1].mVertexType);
      event.mVerts[1].mPosition   = entity.mVerts[1].mPosition;

      WsfDisInterface* disInterface = WsfDisInterface::Find(*mSimulation);
      auto xioPlat = simEnv.GetXIO_Info().GetPlatformInfo(senderPtr, entity.mVerts[0].mRelativePlatformIndex);
      if (xioPlat && disInterface)
      {
         auto disPlat = disInterface->FindDisPlatform(xioPlat->GetEntityId().Convert<DisEntityId>());
         if (disPlat)
         {
            event.mVerts[0].mPlatformIndex = disPlat->GetPlatform()->GetIndex();
         }
      }

      xioPlat = simEnv.GetXIO_Info().GetPlatformInfo(senderPtr, entity.mVerts[1].mRelativePlatformIndex);
      if (xioPlat && disInterface)
      {
         auto disPlat = disInterface->FindDisPlatform(xioPlat->GetEntityId().Convert<DisEntityId>());
         if (disPlat)
         {
            event.mVerts[1].mPlatformIndex = disPlat->GetPlatform()->GetIndex();
         }
      }

      events.emplace_back(event);
   }

   for (const auto& removeId : aPkt.mRemovedIds)
   {
      WsfDraw::DrawEvent event;

      event.mID       = removeId.first;
      event.mDrawType = WsfDraw::DrawType::cERASE;

      events.emplace_back(event);
   }

   AddSimEvent(ut::make_unique<DrawEvent>(mSimulation->GetSimTime(), layer, events));
}
