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

#ifndef PREFOBJECT_HPP
#define PREFOBJECT_HPP

#include <QSet>
#include <QString>

#include "WkfPrefObject.hpp"

namespace ProjectBrowser
{
struct PrefData
{
   QSet<QString> mHiddenFileExtensions{"db", "afproj", "bak", "dll", "exe", "svn"};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "FileOptions";

   PrefObject(QObject* parent = nullptr);
   ~PrefObject() override = default;

   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   QSet<QString> GetHiddenFileExtensions() const;
signals:
   void Changed();
};
} // namespace ProjectBrowser

Q_DECLARE_METATYPE(ProjectBrowser::PrefData)
#endif
