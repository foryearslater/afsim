// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ASTROLABEUI_HPP
#define ASTROLABEUI_HPP

#include <memory>

#include <QDockWidget>

#include "WsfScenario.hpp"

namespace wsfg
{
namespace spaceg
{

class AstrolabeConfig;
class MissionVerificationResults;

} // namespace spaceg
} // namespace wsfg

#include "AstrolabeDockWidgetBase.hpp"
#include "OrbitPreviewManager.hpp"
#include "OrbitalMissionModel.hpp"

namespace SpaceTools
{

//! The UI for Astrolabe.
class AstrolabeUI : public wsfg::spaceg::AstrolabeDockWidgetBase
{
   Q_OBJECT

public:
   explicit AstrolabeUI(const wsfg::spaceg::AstrolabeConfig& aConfig,
                        QWidget*                             aParentPtr = nullptr,
                        Qt::WindowFlags                      aFlags     = Qt::WindowFlags{});
   ~AstrolabeUI() override = default;

   void PopulateSpacePlatforms(const QStringList& aPlatformList);

   const wsfg::spaceg::OrbitalMissionModel* GetMission() const;

public slots:
   void SelectPlatform(const QString& aPlatformName);
   void SetOrbitalMissionModel(wsfg::spaceg::OrbitalMissionModel* aModelPtr);

   void PlatformAdded(const QString& aAddedPlatformName);
   void PlatformRemoved(const QString& aRemovedPlatformName);

signals:
   void PlatformRequested(const QString& aPlatformName);
   void OutputRequested();
   void VerificationRequested();

private:
   void InitialVerifyAction() override;
   void VerifyAction() override;
   void OnPlatformSelected(const QString& aPlatformName);
   void PlatformReload(const QString& aPlatformName);
   void HandleReload();
   void InitialApplyAction() override;
   void ApplyAction() override;

   QToolButton*                 mReloadButtonPtr;
   std::unique_ptr<WsfScenario> mScenarioPtr;
};

} // namespace SpaceTools

#endif // ASTROLABEUI_HPP
