// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DialogBuilderPrefObject_hpp
#define DialogBuilderPrefObject_hpp

class QSettings;
#include "DialogBuilderDialogDefinition.hpp"
#include "DialogBuilderDockWidget.hpp"
#include "WkfPrefObject.hpp"

namespace WkDialogBuilder
{
struct PrefData
{
   QMap<QString, DialogDef> dialogs;
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   PrefObject(QObject* parent = nullptr);

signals:
   void DialogDefinitionChanged();

private:
   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace WkDialogBuilder

Q_DECLARE_METATYPE(WkDialogBuilder::PrefData)
#endif
