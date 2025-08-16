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
#ifndef PATTERNVISUALIZATIONPLUGIN_HPP
#define PATTERNVISUALIZATIONPLUGIN_HPP

#include "DockWidget.hpp"
#include "Plugin.hpp"
#include "PrefWidget.hpp"
#include "Session.hpp"
#include "WsfStandardApplication.hpp"

class QMenu;
class QTemporaryFile;
class WsfPM_Comm;
class WsfPM_Platform;
class WsfPM_Sensor;

namespace PatternVisualizer
{
class Plugin : public wizard::Plugin
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

protected:
private:
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr);
   void RunPatternVisualization(const WsfPProxyNode& aNode);
   void AddFile(QTemporaryFile* aTempFilePtr) { mTempFiles.push_back(aTempFilePtr); }
   void SetupOpenGL();

   void PopulatePlatform(WsfPM_Platform& aPlatform, wizard::Project* aProjectPtr, std::string& aStr);
   bool PopulateTransceiver(const std::string& aName, const WsfPProxyNode& aTcvr, wizard::Project* aProject, std::string& aStr);
   bool PopulateBeam(const std::string& aName, WsfPProxyNode& aBeam, wizard::Project* aProject, std::string& aStr);
   bool PopulateMode(const std::string& aName, WsfPProxyNode& aMode, wizard::Project* aProject, std::string& aStr);
   bool PopulateWeaponMode(const std::string& aName, WsfPProxyNode& aMode, wizard::Project* aProject, std::string& aStr);
   bool PopulateJammer(const std::string& aName, const WsfPProxyNode& aNode, wizard::Project* aProjectPtr, std::string& aStr);
   bool PopulateSensor(WsfPM_Sensor& aSensor, wizard::Project* aProjectPtr, std::string& aStr);
   void PopulatePattern(const std::string&   aName,
                        const WsfPProxyNode& aNode,
                        wizard::Project*     aProjectPtr,
                        std::string&         aStr,
                        const std::string&   aSigviewCat = std::string());
   void PopulateComm(WsfPM_Comm& aComm, wizard::Project* aProjectPtr, std::string& aStr);

   std::vector<QTemporaryFile*> mTempFiles;
   DockWidget*                  mDockWidgetPtr;
   PluginUiPointer<PrefWidget>  mPrefWidgetPtr;
};
} // namespace PatternVisualizer
#endif
