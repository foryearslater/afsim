// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 - 2019 Lockheed Martin Corporation. All Rights Reserved.
// Lockheed Martin Corporation hereby provides the below source code to the
// United States Government with Unlimited Rights as such is defined in
// DFARS 252.227 - 7014.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SIMDIS_IDE_PLUGIN_HPP
#define SIMDIS_IDE_PLUGIN_HPP

#include "Plugin.hpp"

#include <QFileInfo>
#include <QMenu>
#include <QProcess>
#include <QSettings>

#include "Editor.hpp"

namespace wizard
{
   namespace SIMDIS
   {
      class Plugin : public wizard::Plugin
      {
         // this class provides UI in wizard for integration with SIMDIS
         // including configuring the location of SIMDIS
         // launch SIMDIS and
         // and setting models and beam colors.
      public:
         explicit Plugin(const QString& aPluginName,
                         const size_t   aUniqueId);
         ~Plugin() override = default;

         void ActionsForFileRequested(const QFileInfo& aFileInfo,
                                      QMenu*           aMenu);
      private:
         void ConfigureSIMDIS();
         void BuildModelList(const UtPath&      aPath,
                             QMenu&             aMenu);
         void onEditorContextMenu(wizard::Editor*  aEditControlPtr,
                                  QMenu&           aMenu);
         void Launch(const QFileInfo& aPath);
         void LoadSettings(QSettings& aSettings) override;
         void SaveSettings(QSettings& aSettings) override;
         void RecordModels(QMenu*             aMenu,
                           QSettings&         aSettings,
                           int&               aIndex,
                           const QStringList& aPrefix);

         void ReplaceRangeWithColor(const UtTextDocumentRange& aRange);

         QString           mActionName;
         QMenu             mModelMenu;
         QString           mExePath;
         UtCallbackHolder  mCallbacks;

      };
   }
}

#endif
