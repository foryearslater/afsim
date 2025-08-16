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

#ifndef PROXYINTERFACE_HPP
#define PROXYINTERFACE_HPP

#include <QMap>
#include <QObject>

#include "UtPath.hpp"
#include "WsfPProxyNode.hpp"

namespace wizard
{
class ProxyMerge;
}

namespace SPLAT
{
class ProxyInterface : public QObject
{
   Q_OBJECT
public:
   ProxyInterface();
   ~ProxyInterface() override = default;

   void        SetProxyNode(const WsfPProxyNode& aProxyNode) { mProxyNode = aProxyNode; }
   void        ResetProxyNode() { mProxyNode = WsfPProxyNode(); }
   bool        ProxyNodeIsEqualTo(const WsfPProxyNode& aProxyNode) { return mProxyNode == aProxyNode; }
   std::string GetProxyNodeName() const { return mProxyNode.GetName(); }
   UtPath      GetProxyNodeFile() const { return mProxyNode.GetSourceDocumentRange().mSource->GetFilePath(); }
   QStringList GetModeNames() { return mModeNames; }

   QStringList GetPlatformTypes() { return mPlatformTypeFiles.keys(); }
   QStringList GetRadarSignatures() { return mRadarSignatureFiles.keys(); }
   QStringList GetInfraredSignatures() { return mInfraredSignatureFiles.keys(); }
   QStringList GetOpticalSignatures() { return mOpticalSignatureFiles.keys(); }
   QStringList GetAcousticSignatures() { return mAcousticSignatureFiles.keys(); }

   UtPath GetPlatformTypeFile(const QString& aPlatformType) { return mPlatformTypeFiles.value(aPlatformType); }
   UtPath GetRadarSignatureFile(const QString& aRadarSignature) { return mRadarSignatureFiles.value(aRadarSignature); }
   UtPath GetInfraredSignatureFile(const QString& aInfraredSignature)
   {
      return mInfraredSignatureFiles.value(aInfraredSignature);
   }
   UtPath GetOpticalSignatureFile(const QString& aOpticalSignature)
   {
      return mOpticalSignatureFiles.value(aOpticalSignature);
   }
   UtPath GetAcousticSignatureFile(const QString& aAcousticSignature)
   {
      return mAcousticSignatureFiles.value(aAcousticSignature);
   }

   void Update(wizard::ProxyMerge* aProxyMerge);

   bool ContainsPlatformType(const QString& aPlatformType) { return mPlatformTypeFiles.contains(aPlatformType); }
signals:
   void ProxyUpdated();

private:
   WsfPProxyNode mProxyNode;
   QStringList   mModeNames;

   QMap<QString, UtPath> mPlatformTypeFiles;
   QMap<QString, UtPath> mRadarSignatureFiles;
   QMap<QString, UtPath> mInfraredSignatureFiles;
   QMap<QString, UtPath> mOpticalSignatureFiles;
   QMap<QString, UtPath> mAcousticSignatureFiles;
};
} // namespace SPLAT

#endif
