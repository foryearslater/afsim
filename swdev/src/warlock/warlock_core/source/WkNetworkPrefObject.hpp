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

#ifndef WK_NETWORK_PREF_OBJECT_HPP
#define WK_NETWORK_PREF_OBJECT_HPP

#include "warlock_core_export.h"

#include <QHostAddress>

#include "WkfPrefObject.hpp"

namespace warlock
{
namespace net
{
struct PrefData
{
   QHostAddress mAddress{"235.127.113.106"};
   QHostAddress mInterface{"127.0.0.1"};
   int          mPort{2360};
};

class WARLOCK_CORE_EXPORT PrefObject final : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   explicit PrefObject(QObject* aParent = nullptr);
   ~PrefObject() override = default;

   static constexpr const char* cNAME = "Network";

signals:
   void NetworkChanged(const QHostAddress& aAddress, const QHostAddress& aInterface, int aPort);

private:
   void Apply() override;

   void     SetPreferenceDataP(const PrefData& aPrefData) override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   bool mNetworkChanged = true;
};
} // namespace net
} // namespace warlock

Q_DECLARE_METATYPE(warlock::net::PrefData)

#endif
