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

#ifndef ASTROLABEDOCKWIDGET_HPP
#define ASTROLABEDOCKWIDGET_HPP

#include <memory>

#include <QDockWidget>
#include <QSet>
#include <QString>

#include "AstrolabeDockWidgetBase.hpp"
#include "OrbitPreviewManager.hpp"
#include "OrbitalMissionModel.hpp"

namespace wsfg
{
namespace spaceg
{

class AstrolabeConfig;

} // namespace spaceg
} // namespace wsfg

namespace WkAstrolabe
{

class SimInterface;

class DockWidget : public wsfg::spaceg::AstrolabeDockWidgetBase
{
   Q_OBJECT

public:
   DockWidget(const wsfg::spaceg::AstrolabeConfig& aConfig,
              SimInterface*                        aSimInterfacePtr,
              QWidget*                             aParentPtr = nullptr,
              Qt::WindowFlags                      aFlags     = Qt::WindowFlags{});
   ~DockWidget() override = default;

   void SetPlatformAvailability(const QString& aPlatformName, bool aAvailable);

   bool IsRelevantPlatform(const QString& aPlatformName) const;

   void HandlePlatformSelectionRequest(const QString& aPlatformName);

private:
   void SetModel(std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> aMissionModelPtr);
   std::unique_ptr<wsfg::spaceg::OrbitalMissionModel> GetEmptyMission() const;

   void ClearCurrentSelection();

   void CancelMissionSequence();
   void DiscardMissionSequence();
   void InitialVerifyAction() override;
   void VerifyAction() override;
   void InitialApplyAction() override;
   void ApplyAction() override;

   bool VerifyDiscardFromUser();

   void UpdateActionButtons();

   SimInterface* mSimInterfacePtr;
   QSet<QString> mRelevantPlatforms;
};

} // namespace WkAstrolabe

#endif // ASTROLABEDOCKWIDGET_HPP
