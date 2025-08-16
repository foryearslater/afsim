// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "ProjectorPlugin.hpp"

#include <QMenu>

#include "ProjectorSimCommands.hpp"
#include "VaObserver.hpp"
#include "VaWidget.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfSharedMapPreferencesObject.hpp"
#include "WkfUtils.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(Projector::Plugin, "Projector", "Draws projections onto terrain from platforms", "warlock")

Projector::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   mCallbacks.Add(wkf::Observer::EntityDeleted.Connect(&Plugin::PlatformDeletedHandler, this));
}

void Projector::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   std::vector<QAction*> actions;
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      QString        name  = QString::fromStdString(aEntityPtr->GetName());
      wkf::Platform* plat  = dynamic_cast<wkf::Platform*>(aEntityPtr);
      auto           plist = mProjectableSensors.find(name);
      if (plist != mProjectableSensors.end())
      {
         for (auto& p : plist->second)
         {
            if (!ProjectorExists(name, p))
            {
               QAction* projectorAction = new QAction(QString("Add Projector to %1 : %2").arg(name, p), aMenu);
               connect(projectorAction, &QAction::triggered, this, [=]() { CreateProjector(*plat, p); });
               actions.emplace_back(projectorAction);
            }
         }
      }
      auto&& projectorList = mProjectorMap.find(name);
      if (projectorList != mProjectorMap.end())
      {
         for (auto&& projector : projectorList->second)
         {
            QAction* remAction = new QAction(QString("Remove Projector from %1 : %2").arg(name, projector.first), aMenu);
            connect(remAction, &QAction::triggered, this, [=]() { RemoveProjector(*plat, projector.first); });
            actions.emplace_back(remAction);
         }
      }
   }
   if (!actions.empty())
   {
      QMenu* submenu = aMenu->addMenu("Projectors");
      for (auto& act : actions)
      {
         submenu->addAction(act);
      }
   }
}

void Projector::Plugin::CreateProjector(wkf::Platform& aPlatform, const QString& aSensorName)
{
   GetInterface()->AddSimCommand(
      ut::make_unique<InstallProjectorUpdaterCommand>(GetInterface(), aPlatform.GetName(), aSensorName.toStdString()));
}

void Projector::Plugin::RemoveProjector(const wkf::Platform& aPlatform, const QString& aSensorName)
{
   RemoveProjector(QString::fromStdString(aPlatform.GetName()), aSensorName);
}

void Projector::Plugin::RemoveProjector(const QString& aPlatformName, const QString& aSensorName)
{
   GetInterface()->AddSimCommand(ut::make_unique<UninstallProjectorUpdaterCommand>(GetInterface(),
                                                                                   aPlatformName.toStdString(),
                                                                                   aSensorName.toStdString()));
   const auto& platProj = mProjectorMap.find(aPlatformName);
   if (platProj != mProjectorMap.end())
   {
      const auto& sensProj = platProj->second.find(aSensorName);
      if (sensProj != platProj->second.end())
      {
         wkf::Viewer* mainViewer = vaEnv.GetStandardViewer();
         if (mainViewer)
         {
            UtoCmeTerrain* terrain = dynamic_cast<UtoCmeTerrain*>(mainViewer->GetDatabase());
            if (terrain != nullptr)
            {
               terrain->RemoveProjector(sensProj->second);
            }
         }

         platProj->second.erase(sensProj);
      }
      if (platProj->second.empty())
      {
         mProjectorMap.erase(platProj);
      }
   }
}

void Projector::Plugin::PlatformDeletedHandler(vespa::VaEntity* aPlatformPtr)
{
   wkf::Platform* plat = dynamic_cast<wkf::Platform*>(aPlatformPtr);
   if (plat)
   {
      const auto&       projPlat = mProjectorMap.find(QString::fromStdString(plat->GetName()));
      std::set<QString> sensSet;
      if (projPlat != mProjectorMap.end())
      {
         for (auto& sens : projPlat->second)
         {
            sensSet.insert(sens.first);
         }
      }
      for (auto& sens : sensSet)
      {
         RemoveProjector(*plat, sens);
      }
   }
}

void Projector::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(this);
}

bool Projector::Plugin::ProjectorExists(const QString& aPlatformName, const QString& aSensorName) const
{
   const auto& plat = mProjectorMap.find(aPlatformName);
   if (plat != mProjectorMap.end())
   {
      const auto& sens = plat->second.find(aSensorName);
      if (sens != plat->second.end())
      {
         return true;
      }
   }
   return false;
}

void Projector::Plugin::UpdateProjector(const wkf::Platform& aPlatform,
                                        unsigned int         aProjectorId,
                                        const SensorState&   aSensorState)
{
   wkf::Viewer* mainViewer = vaEnv.GetStandardViewer();
   if (mainViewer)
   {
      UtoCmeTerrain* terrain = dynamic_cast<UtoCmeTerrain*>(mainViewer->GetDatabase());
      if (terrain)
      {
         osg::Matrix modelMatrix;
         double      vec[3];
         aPlatform.GetPosition().GetECEF(vec);
         UtLLAPos lla;
         aPlatform.GetPosition().GetLLA(lla);
         double hpr[3];
         aPlatform.GetOrientation(hpr[0], hpr[1], hpr[2]);

         modelMatrix = osg::Matrixd::translate(-vec[0], -vec[1], -vec[2]) *
                       osg::Matrixd::rotate(-osg::PI_2, osg::Vec3d(1, 0, 0)) *
                       osg::Matrixd::rotate(-osg::PI_2, osg::Vec3d(0, 1, 0)) *
                       osg::Matrixd::rotate(osg::DegreesToRadians(-lla.GetLon()), 0.0, 1.0, 0.0) *
                       osg::Matrixd::rotate(osg::DegreesToRadians(lla.GetLat()), 1.0, 0.0, 0.0) *
                       osg::Matrixd::rotate(hpr[0],
                                            osg::Vec3d(0.0, 0.0, 1.0), // heading
                                            -hpr[1],
                                            osg::Vec3d(1.0, 0.0, 0.0), // pitch
                                            -hpr[2],
                                            osg::Vec3d(0.0, 1.0, 0.0)); // roll


         osg::Matrix viewMatrix = osg::Matrixd::rotate(aSensorState.mYaw,
                                                       osg::Vec3(0, 0, 1),
                                                       -aSensorState.mRoll,
                                                       osg::Vec3(0, 1, 0),
                                                       -aSensorState.mPitch - osg::PI_2,
                                                       osg::Vec3(1, 0, 0));

         osg::Matrix mv         = modelMatrix * viewMatrix;
         osg::Matrix projMatrix = osg::Matrixd::perspective(aSensorState.mElWidth * UtMath::cDEG_PER_RAD,
                                                            aSensorState.mElWidth / aSensorState.mAzWidth,
                                                            100.0,
                                                            7000000.0);
         terrain->UpdateProjectorMatrix(aProjectorId, mv, projMatrix);
      }
   }
}

void Projector::Plugin::UpdateSensor(const std::string& aPlatformName,
                                     const std::string& aSensorName,
                                     const SensorState& aSensorState)
{
   QString qpname                                        = QString::fromStdString(aPlatformName);
   QString qsname                                        = QString::fromStdString(aSensorName);
   mSensors[qpname][QString::fromStdString(aSensorName)] = aSensorState;
   if (aSensorState.mUsable)
   {
      if (!ProjectorExists(qpname, qsname))
      {
         wkf::Viewer* mainViewer = vaEnv.GetStandardViewer();
         if (mainViewer)
         {
            UtoCmeTerrain* terrain = dynamic_cast<UtoCmeTerrain*>(mainViewer->GetDatabase());
            if (terrain != nullptr)
            {
               std::string  name  = QString(qpname + "_" + qsname).toStdString();
               UtColor      color = wkf::util::GetHashUtColor(name);
               unsigned int id    = terrain->AddProjector(name, color);
               if (id != 0)
               {
                  mProjectorMap[qpname][qsname] = id;
               }
            }
         }
      }
      const auto& platformsProjectors = mProjectorMap.find(qpname);
      if (platformsProjectors != mProjectorMap.end())
      {
         const auto& projector = platformsProjectors->second.find(qsname);
         if (projector != platformsProjectors->second.end())
         {
            auto scenario = vaEnv.GetStandardScenario();
            if (scenario != nullptr)
            {
               wkf::Platform* plat = scenario->FindPlatformByName(aPlatformName);
               if (plat != nullptr)
               {
                  UpdateProjector(*plat, projector->second, aSensorState);
               }
            }
         }
      }
   }
   else
   {
      const auto& platformsProjectors = mProjectorMap.find(qpname);
      if (platformsProjectors != mProjectorMap.end())
      {
         const auto& projector = platformsProjectors->second.find(qsname);
         if (projector != platformsProjectors->second.end())
         {
            RemoveProjector(aPlatformName, projector->first);
         }
      }
   }
}

void Projector::Plugin::SensorProjectableChanged(const std::string& aPlatformName, const std::string& aSensorName, bool aGood)
{
   QString qpname = QString::fromStdString(aPlatformName);
   QString qsname = QString::fromStdString(aSensorName);
   if (aGood)
   {
      mProjectableSensors[qpname].insert(qsname);
   }
   else
   {
      auto slist = mProjectableSensors.find(qpname);
      if (slist != mProjectableSensors.end())
      {
         slist->second.erase(qsname);
         if (slist->second.empty())
         {
            mProjectableSensors.erase(qpname);
         }
      }
   }
}
