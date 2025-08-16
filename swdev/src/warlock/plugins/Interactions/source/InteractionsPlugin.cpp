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
#include "InteractionsPlugin.hpp"

#include "WkfEnvironment.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkInteractions::Plugin,
   "Interactions",
   "The Interactions plugin displays lines between platforms on the map to depict interactions within the simulation. "
   "Interactions include detections, tracks, jamming, communications, tasking, and weapon engagements.",
   "warlock")

WkInteractions::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface, wkf::InteractionPluginBase>(aPluginName, aUniqueId)
   , mViewerPtr(nullptr)
{
   connect(GetPrefObject(), &wkf::InteractionPrefObject::TimeoutChanged, GetInterface(), &SimInterface::SetTimeout);
   connect(&wkfEnv,
           &wkf::Environment::Initialize,
           this,
           [this]()
           {
              mViewerPtr = vaEnv.GetStandardViewer();
              mInterfacePtr->SetEnabled(mViewerPtr != nullptr);
           });
   RegisterStateCard("CyberDetect", ":/icons/detect.png", "Cyber");
   RegisterStateCard("CyberImmune", ":/icons/immune.png", "Cyber");
   RegisterStateCard("CyberCompromise", ":/icons/vulnerable.png", "Cyber");
   RegisterStateCard("CyberDamage", ":/icons/damage.png", "Cyber");
   RegisterInteraction("CyberAttributed", "Cyber", "cyber-attributed", QColor(0, 255, 255));
   RegisterInteraction("CyberScan", "Cyber", "cyber-scans", QColor(0, 255, 192));
   RegisterInteraction("CyberAttack", "Cyber", "cyber-attacks", QColor(255, 143, 143));
}

void WkInteractions::Plugin::GuiUpdate()
{
   if (mViewerPtr)
   {
      // Process all the new SimEvents
      mInterfacePtr->ProcessEvents(*mViewerPtr, GetPrefObject());
   }
}
