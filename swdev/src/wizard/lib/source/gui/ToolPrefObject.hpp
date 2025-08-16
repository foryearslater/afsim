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

#ifndef TOOLPREFOBJECT_HPP
#define TOOLPREFOBJECT_HPP

#include "ViExport.hpp"
#include "WkfPrefObject.hpp"

namespace wizard
{
struct ToolDef
{
   ToolDef(const QString&     aAlias,
           const QString&     aPath,
           const QString&     aCmdLine,
           const QString&     aWorking,
           const QStringList& aExtensions)
      : mAlias(aAlias)
      , mPath(aPath)
      , mCmdLine(aCmdLine)
      , mWorking(aWorking)
      , mExtensions(aExtensions)
   {
   }
   ToolDef() = default;

   QString     mAlias;
   QString     mPath;
   QString     mCmdLine;
   QString     mWorking;
   QStringList mExtensions;
};
struct ToolPrefData
{
   ToolPrefData();

   std::map<QString, ToolDef> mToolList;
};

class VI_EXPORT ToolPrefObject : public wkf::PrefObjectT<ToolPrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "ToolLauncher";

   ToolPrefObject(QObject* parent = nullptr);
   ~ToolPrefObject() override = default;

   void Apply() override;

   ToolPrefData ReadSettings(QSettings& aSettings) const override;
   void         SaveSettingsP(QSettings& aSettings) const override;

   const ToolDef                     FindTool(const QString& aName) const;
   const std::map<QString, ToolDef>& Tools() const;

signals:
   void Changed();
};
} // namespace wizard

Q_DECLARE_METATYPE(wizard::ToolPrefData)

#endif
