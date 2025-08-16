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
#ifndef CHATPREFOBJECT_HPP
#define CHATPREFOBJECT_HPP

#include "WkfPrefObject.hpp"

namespace Chat
{
struct PrefData
{
   QString mUserName{"NewUser"};
   bool    mWallTimes{true};
   bool    mSimTimes{true};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   PrefObject(QObject* parent = nullptr);
   ~PrefObject() override = default;

signals:
   void UserNameChanged(const QString& aUserName);
   void TimeStampsChanged(bool aWallTimes, bool aSimTimes);

private:
   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace Chat

Q_DECLARE_METATYPE(Chat::PrefData)
#endif
