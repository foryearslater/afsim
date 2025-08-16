// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef LOGSERVERPREFOBJECT_HPP
#define LOGSERVERPREFOBJECT_HPP

class QSettings;
#include "ViExport.hpp"
#include "WkfPrefObject.hpp"

namespace LogServer
{
struct PrefData
{
   int mLogServerPort = 18888;
};

class VI_EXPORT PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT
public:
   static constexpr const char* cNAME = "LogServer";
   PrefObject(QObject* parent = nullptr);
   int GetLogServerPort() const { return mCurrentPrefs.mLogServerPort; }

private:
   void     EmitAllSignals();
   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace LogServer

Q_DECLARE_METATYPE(LogServer::PrefData)
#endif // LOGSERVERPREFOBJECT_HPP
