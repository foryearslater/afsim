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

#include "PatternVisualizationPlugin.hpp"

#include <QMenu>
#include <QProcess>
#include <QTemporaryFile>

#include "ParseResults.hpp"
#include "Project.hpp"
#include "Signals.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfPM_MilPlatform.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPM_Root.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(PatternVisualizer::Plugin,
                          "Pattern Visualization",
                          "Allows the visualization of patterns from context menus on appropriate simulation objects.",
                          "wizard")

PatternVisualizer::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wizard::Plugin(aPluginName, aUniqueId)
   , mDockWidgetPtr(nullptr)
   , mPrefWidgetPtr(new PrefWidget)
{
   connect(wizSignals, &wizard::Signals::ActionsForNodeRequested, this, &Plugin::ActionsForNodeRequested);
}

PatternVisualizer::Plugin::~Plugin()
{
   for (auto&& it : mTempFiles)
   {
      delete it;
   }
}

void PatternVisualizer::Plugin::ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr)
{
   QAction* actionPtr = nullptr;

   // Add an action to the menu depending on type of node selected
   if (aNode.IsOfType("RadarSignature") || aNode.IsOfType("AcousticSignature") || aNode.IsOfType("InfraredSignature") ||
       aNode.IsOfType("OpticalSignature"))
   {
      actionPtr = aMenuPtr->addAction("Visualize Signature...");
   }
   else if (aNode.IsOfType("AntennaPattern"))
   {
      actionPtr = aMenuPtr->addAction("Visualize Antenna Pattern...");
   }
   else if (aNode.IsOfType("Sensor"))
   {
      actionPtr = aMenuPtr->addAction("Visualize Antenna Patterns...");
   }
   else if (aNode.IsOfType("WSF_RF_JAMMER"))
   {
      actionPtr = aMenuPtr->addAction("Visualize Antenna Patterns...");
   }
   /*   else if (aNode.IsOfType("Comm"))
      {
         actionPtr = aMenuPtr->addAction("Visualize Antenna Patterns...");
      }*/ // these are not sufficiently represents in the proxy to draw
   else if (aNode.IsOfType("Platform") && !aNode.GetTypeName().empty())
   {
      actionPtr = aMenuPtr->addAction("Visualize Signatures and Antenna Patterns...");
   }

   // Connect the menu actions
   if (actionPtr != nullptr)
   {
      QObject::connect(actionPtr, &QAction::triggered, [aNode, this]() { RunPatternVisualization(aNode); });
   }
}

void PatternVisualizer::Plugin::RunPatternVisualization(const WsfPProxyNode& aNode)
{
   // get unique filename
   std::string      forPatternViewer = "";
   wizard::Project* projectPtr       = wizard::Project::Instance();

   // platform
   if (aNode.IsOfType("Platform"))
   {
      if (projectPtr && projectPtr->GetParseResults())
      {
         WsfPM_Platform plat(aNode);
         PopulatePlatform(plat, projectPtr, forPatternViewer);
      }
   }
   else if (aNode.IsOfType("Sensor"))
   {
      if (projectPtr && projectPtr->GetParseResults())
      {
         WsfPM_Sensor sensor(aNode);
         PopulateSensor(sensor, projectPtr, forPatternViewer);
      }
   }
   else if (aNode.IsOfType("Comm"))
   {
      if (projectPtr && projectPtr->GetParseResults())
      {
         WsfPM_Comm comm(aNode);
         PopulateComm(comm, projectPtr, forPatternViewer);
      }
   }
   else if (aNode.IsOfType("WSF_RF_JAMMER"))
   {
      if (projectPtr && projectPtr->GetParseResults())
      {
         PopulateJammer("", aNode, projectPtr, forPatternViewer);
      }
   }
   else // antenna or signature
   {
      if (projectPtr && projectPtr->GetParseResults())
      {
         PopulatePattern("", aNode, projectPtr, forPatternViewer);
      }
   }

   // Create a temporary file
   QDir            tmpDir           = QDir::tempPath();
   QString         tempFileTemplate = tmpDir.filePath(QString("AFSIM_Wizard.XXXXXX"));
   QTemporaryFile* tempFilePtr      = new QTemporaryFile(tempFileTemplate);
   if (tempFilePtr->open())
   {
      tempFilePtr->write(forPatternViewer.c_str());
      tempFilePtr->flush();
      tempFilePtr->close();
      Plugin::AddFile(tempFilePtr);

      SetupOpenGL();

      if (mDockWidgetPtr == nullptr)
      {
         mDockWidgetPtr = new DockWidget("Pattern Visualizer", wkfEnv.GetMainWindow()->centralWidget());
      }
      else
      {
         mDockWidgetPtr->ClearAllPatterns();
      }

      mDockWidgetPtr->ReadPatternFile(wizard::Project::Instance()->GetStartupFiles()[0].GetSystemPath().c_str(),
                                      tempFilePtr->fileName());
   }
   else
   {
      delete tempFilePtr;
   }
}

void PatternVisualizer::Plugin::SetupOpenGL()
{
   auto format = QSurfaceFormat{};
   format.setDepthBufferSize(24);
   format.setStencilBufferSize(8);
   format.setSamples(4);
   format.setVersion(3, 3);
   format.setRenderableType(QSurfaceFormat::OpenGL);
   format.setProfile(QSurfaceFormat::CoreProfile);
   format.setOption(QSurfaceFormat::DebugContext);

   QSurfaceFormat::setDefaultFormat(format);
}

void PatternVisualizer::Plugin::PopulatePlatform(WsfPM_Platform& aPlatform, wizard::Project* aProjectPtr, std::string& aStr)
{
   std::string platformStr;
   platformStr += "sigview_category ";
   platformStr += aPlatform.GetName();
   platformStr += "\n";
   bool            addedPlatform = false;
   WsfPM_SensorMap sensorMap     = aPlatform.Sensors();
   if (sensorMap.IsValid())
   {
      bool                       sensorsAdded = false;
      std::vector<WsfPProxyNode> types;
      sensorMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            WsfPM_Sensor sensor(*typeit);
            std::string  sensorStr;
            if (PopulateSensor(sensor, aProjectPtr, sensorStr))
            {
               sensorsAdded = true;
               if (!addedPlatform)
               {
                  aStr += platformStr;
                  aStr += "sigview_category Sensors\n";
                  addedPlatform = true;
               }
               aStr += sensorStr;
            }
         }
      }
      if (sensorsAdded)
      {
         aStr += "end_sigview_category\n"; // end sensors
      }
   }
   WsfPM_MilPlatform mp(aPlatform);
   WsfPM_WeaponMap   weaponMap = mp.Weapons();
   if (weaponMap.IsValid())
   {
      bool                       jammersAdded = false;
      std::vector<WsfPProxyNode> types;
      weaponMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            if (typeit->IsOfType("WSF_RF_JAMMER"))
            {
               std::string jammerStr;
               if (PopulateJammer("", *typeit, aProjectPtr, jammerStr))
               {
                  jammersAdded = true;
                  if (!addedPlatform)
                  {
                     aStr += platformStr;
                     aStr += "sigview_category Jammers\n";
                     addedPlatform = true;
                  }
                  aStr += jammerStr;
               }
            }
         }
      }
      if (jammersAdded)
      {
         aStr += "end_sigview_category\n"; // end jammers
      }
   }
   // comm should be collected here, once it is sufficiently built up in the proxy
   WsfPM_Root proot(aPlatform.GetProxy());

   std::string   sigsStr;
   WsfPProxyNode pn = proot.radarSignatures().Find(aPlatform.RadarSignature());
   PopulatePattern("radar_signature", pn, aProjectPtr, sigsStr);

   pn = proot.opticalSignatures().Find(aPlatform.OpticalSignature());
   PopulatePattern("optical_signature", pn, aProjectPtr, sigsStr);

   pn = proot.infraredSignatures().Find(aPlatform.InfraredSignature());
   PopulatePattern("infrared_signature", pn, aProjectPtr, sigsStr);

   pn = proot.acousticSignatures().Find(aPlatform.AcousticSignature());
   PopulatePattern("acoustic_signature", pn, aProjectPtr, sigsStr);

   if (!sigsStr.empty())
   {
      if (!addedPlatform)
      {
         aStr += platformStr;
         addedPlatform = true;
      }
      aStr += "sigview_category Signatures\n";
      aStr += sigsStr;
      aStr += "end_sigview_category\n";
   }

   if (addedPlatform)
   {
      aStr += "end_sigview_category\n";
   }
}

bool PatternVisualizer::Plugin::PopulateTransceiver(const std::string&   aName,
                                                    const WsfPProxyNode& aTcvr,
                                                    wizard::Project*     aProject,
                                                    std::string&         aStr)
{
   std::string pattern = aTcvr.GetValue().GetAttr("antennaPattern").ValueToString();
   if (!pattern.empty())
   {
      aStr += "sigview_category ";
      aStr += aName;
      aStr += "\n";
      WsfPM_Root proot(aTcvr.GetProxy());

      WsfPProxyNode pn = proot.antennaPatterns().Find(pattern);
      PopulatePattern(aName, pn, aProject, aStr);
      aStr += "end_sigview_category\n";
      return true;
   }
   return false;
}

bool PatternVisualizer::Plugin::PopulateBeam(const std::string& aName,
                                             WsfPProxyNode&     aBeam,
                                             wizard::Project*   aProject,
                                             std::string&       aStr)
{
   bool                 added    = false;
   const WsfPProxyNode& xmtrRcvr = aBeam + "xmtrRcvr";
   if (xmtrRcvr.IsValid())
   {
      const WsfPProxyNode& xmtr = xmtrRcvr + "transmitter";
      if (xmtr.IsValid())
      {
         added |= PopulateTransceiver("transmitter", xmtr, aProject, aStr);
      }
      const WsfPProxyNode& rcvr = xmtrRcvr + "receiver";
      if (rcvr.IsValid())
      {
         added |= PopulateTransceiver("receiver", rcvr, aProject, aStr);
      }
   }
   else
   {
      const WsfPProxyNode& rcvr = aBeam + "rcvr";
      if (rcvr.IsValid())
      {
         const WsfPProxyNode& receiver = rcvr + "receiver";
         if (receiver.IsValid())
         {
            const WsfPProxyNode& receiver2 = receiver + "receiver";
            if (receiver2.IsValid()) // there are two layers of "receiver" here
            {
               added |= PopulateTransceiver("receiver", receiver2, aProject, aStr);
            }
            else // if ever we remove the redundant layer, this should work
            {
               added |= PopulateTransceiver("receiver", receiver, aProject, aStr);
            }
         }
      }
   }
   return added;
}

bool PatternVisualizer::Plugin::PopulateMode(const std::string& aName,
                                             WsfPProxyNode&     aMode,
                                             wizard::Project*   aProject,
                                             std::string&       aStr)
{
   bool addedBeam        = false;
   bool addedTransceiver = false;

   WsfPM_ObjectMap beamMap = aMode + "beams";
   if (beamMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      beamMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            addedBeam |= PopulateBeam(typeit->GetName(), *typeit, aProject, aStr);
         }
      }
   }
   WsfPProxyNode beam = aMode + "beam";
   if (beam.IsValid())
   {
      addedBeam |= PopulateBeam("default_beam", beam, aProject, aStr);
   }

   // there are no beams on some sensors... so look here for xmtrrcvr, too
   const WsfPProxyNode& xmtrRcvr = aMode + "xmtrRcvr";
   if (xmtrRcvr.IsValid())
   {
      const WsfPProxyNode& xmtr = xmtrRcvr + "transmitter";
      if (xmtr.IsValid())
      {
         addedTransceiver |= PopulateTransceiver("transmitter", xmtr, aProject, aStr);
      }
      const WsfPProxyNode& rcvr = xmtrRcvr + "receiver";
      if (rcvr.IsValid())
      {
         addedTransceiver |= PopulateTransceiver("receiver", rcvr, aProject, aStr);
      }
   }
   return (addedBeam || addedTransceiver);
}

bool PatternVisualizer::Plugin::PopulateWeaponMode(const std::string& aName,
                                                   WsfPProxyNode&     aMode,
                                                   wizard::Project*   aProject,
                                                   std::string&       aStr)
{
   bool            addedBeam        = false;
   bool            addedTransceiver = false;
   WsfPM_ObjectMap beamMap          = aMode + "beams";
   if (beamMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      beamMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            addedBeam |= PopulateBeam(typeit->GetName(), *typeit, aProject, aStr);
         }
      }
   }
   WsfPProxyNode beam = aMode + "beam";
   if (beam.IsValid())
   {
      addedBeam |= PopulateBeam("default_beam", beam, aProject, aStr);
   }

   // there are no beams on some sensors... so look here for xmtrrcvr, too
   const WsfPProxyNode& xmtrRcvr = aMode + "xmtrAntenna";
   if (xmtrRcvr.IsValid())
   {
      const WsfPProxyNode& xmtr = xmtrRcvr + "transmitter";
      if (xmtr.IsValid())
      {
         addedTransceiver |= PopulateTransceiver("transmitter", xmtr, aProject, aStr);
      }
   }
   return (addedBeam || addedTransceiver);
}

bool PatternVisualizer::Plugin::PopulateJammer(const std::string&   aName,
                                               const WsfPProxyNode& aNode,
                                               wizard::Project*     aProjectPtr,
                                               std::string&         aStr)
{
   std::string jammerStr;
   jammerStr += "sigview_category ";
   jammerStr += aNode.GetName();
   jammerStr += "\n";

   bool          addedJammer  = false;
   WsfPProxyNode modeTemplate = aNode + "template";
   if (modeTemplate.IsValid())
   {
      std::string tmpStr;
      if (PopulateWeaponMode("mode_template", modeTemplate, aProjectPtr, tmpStr))
      {
         aStr += jammerStr;
         aStr += tmpStr;
         addedJammer = true;
      }
   }

   // cycle through modes
   WsfPM_ObjectMap modeMap = aNode + "mode";
   if (modeMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      modeMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            std::string tmpStr;
            if (PopulateWeaponMode(typeit->GetName(), *typeit, aProjectPtr, tmpStr))
            {
               if (!addedJammer)
               {
                  aStr += jammerStr;
                  addedJammer = true;
               }
               aStr += tmpStr;
            }
         }
      }
   }
   if (addedJammer)
   {
      aStr += "end_sigview_category\n";
   }
   return addedJammer;
}

bool PatternVisualizer::Plugin::PopulateSensor(WsfPM_Sensor& aSensor, wizard::Project* aProjectPtr, std::string& aStr)
{
   std::string sensorStr;
   sensorStr += "sigview_category ";
   sensorStr += aSensor.GetName();
   sensorStr += "\n";

   bool          addedSensor  = false;
   WsfPProxyNode modeTemplate = aSensor + "template";
   if (modeTemplate.IsValid())
   {
      std::string tmpStr;
      if (PopulateMode("mode_template", modeTemplate, aProjectPtr, tmpStr))
      {
         aStr += sensorStr;
         aStr += tmpStr;
         addedSensor = true;
      }
   }

   // cycle through modes
   WsfPM_ObjectMap modeMap = aSensor + "mode";
   if (modeMap.IsValid())
   {
      std::vector<WsfPProxyNode> types;
      modeMap.GetAllValues(types);
      std::vector<WsfPProxyNode>::iterator typeit;
      for (typeit = types.begin(); typeit != types.end(); ++typeit)
      {
         if (typeit->IsValid())
         {
            std::string tmpStr;
            if (PopulateMode(typeit->GetName(), *typeit, aProjectPtr, tmpStr))
            {
               if (!addedSensor)
               {
                  aStr += sensorStr;
                  addedSensor = true;
               }
               aStr += tmpStr;
            }
         }
      }
   }
   if (addedSensor)
   {
      aStr += "end_sigview_category\n";
   }
   return addedSensor;
}

void PatternVisualizer::Plugin::PopulatePattern(const std::string&   aName,
                                                const WsfPProxyNode& aNode,
                                                wizard::Project*     aProjectPtr,
                                                std::string&         aStr,
                                                const std::string&   aSigviewCat)
{
   UtTextDocumentRange pos = aProjectPtr->GetParseResults()->FindDefinitionLocation(aNode.GetPath());
   //   aStr += pos.Text();
   //   aStr += "\n";
   std::string text = pos.Text();
   if (!text.empty())
   {
      if (!aSigviewCat.empty())
      {
         aStr += "sigview_category ";
         aStr += aSigviewCat;
         aStr += "\n";
      }
      aStr += text;
      aStr += "\n";
      if (!aSigviewCat.empty())
      {
         aStr += "end_sigview_category\n";
      }
   }
}

void PatternVisualizer::Plugin::PopulateComm(WsfPM_Comm& aComm, wizard::Project* aProjectPtr, std::string& aStr)
{
   // there are no beams on some sensors... so look here for xmtrrcvr, too
   /*   const WsfPProxyNode& xmtrRcvr = aComm + "xmtrRcvr";
      if (xmtrRcvr.IsValid())
      {
         const WsfPProxyNode& xmtr = xmtrRcvr + "transmitter";
         if (xmtr.IsValid())
         {
            PopulateTransceiver("transmitter", xmtr, aProjectPtr, aStr);
         }
         const WsfPProxyNode& rcvr = xmtrRcvr + "receiver";
         if (rcvr.IsValid())
         {
            PopulateTransceiver("receiver", rcvr, aProjectPtr, aStr);
         }
      }*/ // these are not sufficiently represented in the proxy to display
}
