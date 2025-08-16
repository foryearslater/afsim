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
#include "DialogBuilderPrefObject.hpp"

#include <QSettings>

WkDialogBuilder::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "DialogBuildupOptions")
{
}

void WkDialogBuilder::PrefObject::Apply()
{
   // Currently we emit this every time Apply() is triggered,
   //  should we create a dirty bit and only emit when data in dialogs change?
   emit DialogDefinitionChanged();
}

WkDialogBuilder::PrefData WkDialogBuilder::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   int      count1 = aSettings.beginReadArray("dialogs");
   for (int i = 0; i < count1; ++i)
   {
      aSettings.setArrayIndex(i);

      DialogDef dialog;
      dialog.cols       = aSettings.value("cols").toInt();
      dialog.dockable   = aSettings.value("dockable").toBool();
      dialog.name       = aSettings.value("name").toString();
      dialog.rows       = aSettings.value("rows").toInt();
      dialog.buttonSize = (ButtonSize)aSettings.value("buttonSize").toInt();

      int count2 = aSettings.beginReadArray("scripts");
      for (int j = 0; j < count2; ++j)
      {
         aSettings.setArrayIndex(j);

         ScriptDefinition script;
         script.col                    = aSettings.value("col").toInt();
         script.color                  = aSettings.value("color").value<QColor>();
         script.context                = (warlock::script::Context)aSettings.value("context").toInt();
         script.displayName            = aSettings.value("displayName").toString();
         script.icon                   = aSettings.value("icon").toString();
         script.link                   = aSettings.value("link").toString();
         script.platformType           = aSettings.value("platformType").toString();
         script.promptUser             = aSettings.value("promptUser").toBool();
         script.row                    = aSettings.value("row").toInt();
         script.scriptName             = aSettings.value("scriptName").toString();
         script.useDefaultColor        = aSettings.value("useDefaultColor").toBool();
         script.clampPlatformSelection = aSettings.value("clampPlatformSelection").toBool();

         int count3 = aSettings.beginReadArray("args");
         for (int k = 0; k < count3; ++k)
         {
            aSettings.setArrayIndex(k);

            warlock::script::Argument arg;
            arg.data = aSettings.value("value").toString();

            warlock::script::Filter filter;
            filter.isFiltered  = aSettings.value("isFiltered").toBool();
            filter.isInclusive = aSettings.value("isInclusive").toBool();
            filter.type        = aSettings.value("filterType").toString();
            filter.value       = aSettings.value("filterValue").toString();
            script.args.push_back(arg);
            script.filters.push_back(filter);
         }
         aSettings.endArray();
         dialog.scripts.push_back(script);
      }
      aSettings.endArray();
      pData.dialogs.insert(dialog.name, dialog);
   }
   aSettings.endArray();
   return pData;
}

void WkDialogBuilder::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.beginWriteArray("dialogs");
   int i = 0;
   for (auto& it1 : mCurrentPrefs.dialogs)
   {
      aSettings.setArrayIndex(i);

      aSettings.setValue("cols", it1.cols);
      aSettings.setValue("dockable", it1.dockable);
      aSettings.setValue("name", it1.name);
      aSettings.setValue("rows", it1.rows);
      aSettings.setValue("buttonSize", it1.buttonSize);

      int j = 0;
      aSettings.beginWriteArray("scripts");
      for (auto& it2 : it1.scripts)
      {
         aSettings.setArrayIndex(j);

         aSettings.setValue("col", it2.col);
         aSettings.setValue("color", it2.color);
         aSettings.setValue("context", it2.context);
         aSettings.setValue("displayName", it2.displayName);
         aSettings.setValue("icon", it2.icon);
         aSettings.setValue("link", it2.link);
         aSettings.setValue("platformType", it2.platformType);
         aSettings.setValue("promptUser", it2.promptUser);
         aSettings.setValue("row", it2.row);
         aSettings.setValue("scriptName", it2.scriptName);
         aSettings.setValue("useDefaultColor", it2.useDefaultColor);
         aSettings.setValue("clampPlatformSelection", it2.clampPlatformSelection);

         int k = 0;
         aSettings.beginWriteArray("args");
         for (auto& it3 : it2.args)
         {
            aSettings.setArrayIndex(k);

            aSettings.setValue("value", it3.data);
            aSettings.setValue("isFiltered", it2.filters[k].isFiltered);
            aSettings.setValue("isInclusive", it2.filters[k].isInclusive);
            aSettings.setValue("filterType", it2.filters[k].type);
            aSettings.setValue("filterValue", it2.filters[k].value);
            ++k;
         }
         aSettings.endArray();
         ++j;
      }
      aSettings.endArray();
      ++i;
   }
   aSettings.endArray();
}
