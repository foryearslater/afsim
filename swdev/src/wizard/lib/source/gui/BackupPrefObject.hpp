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

#ifndef BACKUPPREFOBJECT_HPP
#define BACKUPPREFOBJECT_HPP

#include "WkfPrefObject.hpp"

namespace wizard
{
struct BackupPrefData
{
   bool mEnable{false};
};

class BackupPrefObject : public wkf::PrefObjectT<BackupPrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "BackupOptions";

   BackupPrefObject(QObject* parent = nullptr);
   ~BackupPrefObject() override = default;

   void Apply() override;

   BackupPrefData ReadSettings(QSettings& aSettings) const override;
   void           SaveSettingsP(QSettings& aSettings) const override;

   bool Enabled() const;
signals:
   void Changed();
};
} // namespace wizard

Q_DECLARE_METATYPE(wizard::BackupPrefData)
#endif
