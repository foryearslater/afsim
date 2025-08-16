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
#ifndef CYBERENGAGEMENTBROWSERPREFOBJECT_HPP
#define CYBERENGAGEMENTBROWSERPREFOBJECT_HPP

class QSettings;
#include "CyberEngagementBrowserDockWidget.hpp"
#include "WkfPrefObject.hpp"

namespace WkCyberEngagementBrowser
{
struct PrefData
{
   int  mPurgeTime{30};
   bool mPurge{false};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   explicit PrefObject(QObject* parent = nullptr);

signals:
   void UpdatePurgeInfo(int aPurgeTime, bool aPurge);

private:
   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace WkCyberEngagementBrowser

Q_DECLARE_METATYPE(WkCyberEngagementBrowser::PrefData)
#endif
