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

#include "ProxyInterface.hpp"

#include "ProjectWorkspace.hpp"
#include "ProxyMerge.hpp"
#include "WsfPM_RadarSensor.hpp"
#include "WsfPM_Root.hpp"

SPLAT::ProxyInterface::ProxyInterface()
{
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProxyUpdated, this, &ProxyInterface::Update);
}

void SPLAT::ProxyInterface::Update(wizard::ProxyMerge* aProxyMerge)
{
   if (aProxyMerge)
   {
      mPlatformTypeFiles.clear();
      mRadarSignatureFiles.clear();
      mInfraredSignatureFiles.clear();
      mOpticalSignatureFiles.clear();
      mAcousticSignatureFiles.clear();
      mModeNames.clear();

      WsfPM_Root        root(mProxyNode.GetProxy());
      WsfPM_RadarSensor sensor(mProxyNode);

      WsfPM_PlatformMap platformTypes      = root.platformTypes();
      WsfPM_ObjectMap   radarSignatures    = root.radarSignatures();
      WsfPM_ObjectMap   infraredSignatures = root.infraredSignatures();
      WsfPM_ObjectMap   opticalSignatures  = root.opticalSignatures();
      WsfPM_ObjectMap   acousticSignatures = root.acousticSignatures();

      WsfPM_RadarSensorModeMap modes = sensor.modes();

      // Sensor Modes
      mModeNames.push_back("");
      for (const auto& m : modes.GetAllObjectNames())
      {
         mModeNames.push_back(m.c_str());
      }

      // Platform Types
      for (const auto& p : platformTypes.GetAllObjectNames())
      {
         UtTextDocumentRange platformTypeDoc = platformTypes.Find(p).GetSourceDocumentRange();
         if (platformTypeDoc.Valid())
         {
            mPlatformTypeFiles.insert(p.c_str(), platformTypeDoc.mSource->GetFilePath());
         }
      }

      // Signatures
      for (const auto& r : radarSignatures.GetAllObjectNames())
      {
         UtTextDocumentRange radarDoc = radarSignatures.Find(r).GetSourceDocumentRange();
         if (radarDoc.Valid())
         {
            mRadarSignatureFiles.insert(r.c_str(), radarDoc.mSource->GetFilePath());
         }
      }

      for (const auto& i : infraredSignatures.GetAllObjectNames())
      {
         UtTextDocumentRange infraredDoc = infraredSignatures.Find(i).GetSourceDocumentRange();
         if (infraredDoc.Valid())
         {
            mInfraredSignatureFiles.insert(i.c_str(), infraredDoc.mSource->GetFilePath());
         }
      }

      for (const auto& o : opticalSignatures.GetAllObjectNames())
      {
         UtTextDocumentRange opticalDoc = opticalSignatures.Find(o).GetSourceDocumentRange();
         if (opticalDoc.Valid())
         {
            mOpticalSignatureFiles.insert(o.c_str(), opticalDoc.mSource->GetFilePath());
         }
      }

      for (const auto& a : acousticSignatures.GetAllObjectNames())
      {
         UtTextDocumentRange acousticDoc = acousticSignatures.Find(a).GetSourceDocumentRange();
         if (acousticDoc.Valid())
         {
            mAcousticSignatureFiles.insert(a.c_str(), acousticDoc.mSource->GetFilePath());
         }
      }

      emit ProxyUpdated();
   }
}
