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
#include "WsfDrawPlugin.hpp"

#include <QAction>
#include <QMenu>

#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "VaViewer.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkWsfDraw::Plugin,
                          "WsfDraw",
                          "The WsfDraw plugin displays WSF Draw commands on the main map display.",
                          "warlock")

WkWsfDraw::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mDrawObjectPtr(new WsfDrawObject(this))
{
   auto mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidgetPtr     = new wkf::WsfDrawDockWidget(mainWindowPtr);
   mDockWidgetPtr->SetModel(*mDrawObjectPtr);

   connect(mDockWidgetPtr,
           &warlock::WsfDrawDockWidget::NetworkCheckboxChanged,
           [this](bool checked) { this->GetInterface()->ReceiveNetworkDraws(checked); });

   mDockWidgetPtr->hide();
}

void WkWsfDraw::Plugin::BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr)
{
   UtVariant v = aViewerPtr->GetData("type");
   if (!v.IsUnset())
   {
      std::string type = v.GetString();
      if (type == "immersive")
      {
         vespa::VaCameraMotionTethered* tether =
            dynamic_cast<vespa::VaCameraMotionTethered*>(aViewerPtr->GetCamera()->GetCameraMotion());
         if (tether)
         {
            wkf::Platform* plat = dynamic_cast<wkf::Platform*>(tether->GetEntity());
            if (plat)
            {
               QList<QString> layerNames = mDrawObjectPtr->GetLayerStrings(aViewerPtr);
               if (!layerNames.empty())
               {
                  QMenu* drawMenu = new QMenu("WsfDraw", aMenu);
                  for (const auto& it : layerNames)
                  {
                     QString  layerName   = it;
                     QAction* layerAction = new QAction(layerName, drawMenu);
                     layerAction->setCheckable(true);
                     layerAction->setChecked(mDrawObjectPtr->GetLayerVisibility(aViewerPtr, layerName));
                     connect(layerAction,
                             &QAction::triggered,
                             [aViewerPtr, layerName, this](bool checked)
                             { mDrawObjectPtr->ShowViewerLayer(aViewerPtr, layerName, checked); });
                     drawMenu->addAction(layerAction);
                  }
                  aMenu->addMenu(drawMenu);
               }
            }
         }
      }
   }
}

void WkWsfDraw::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*mDrawObjectPtr);
}
