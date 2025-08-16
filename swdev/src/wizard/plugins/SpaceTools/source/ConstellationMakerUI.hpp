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

#ifndef CONSTELLATIONMAKERUI_HPP
#define CONSTELLATIONMAKERUI_HPP

#include <QDialog>

#include "ui_SpaceToolsConstellationMaker.h"

class WsfConstellationOptions;

namespace SpaceTools
{

//! The constellation maker dialog box.
class ConstellationMakerUI : public QDialog
{
   Q_OBJECT

public:
   explicit ConstellationMakerUI();
   ~ConstellationMakerUI() override = default;

   void LoadValues(const WsfConstellationOptions& aOptions);
   void SaveValues(WsfConstellationOptions& aOptions);

   void showEvent(QShowEvent* aEvent) override;
   void hideEvent(QHideEvent* aEvent) override;


public slots:
   void NameChanged(const QString& aName);
   void PathChanged(const QString& aPath);
   void AcceptConstellation();
   void AcceptGenerator();
   void RecomputeConjunction();

signals:
   void GenerateConstellation();
   void GenerateGenerator();

private:
   void UpdateConjunctionWarning(bool aWarn);
   void SetValidators();
   bool ValidateInput();
   bool CheckPlatformTypeHasSpaceMover(const QString& aPlatformType);

   Ui::SpaceToolsConstellationMaker mUI;
   QString                          mPath;
};

} // namespace SpaceTools

#endif // CONSTELLATIONMAKERUI_HPP
