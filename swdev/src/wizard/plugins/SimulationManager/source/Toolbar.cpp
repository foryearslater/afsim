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

#include "Toolbar.hpp"

#include <QGuiApplication>

#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WsfExeManager.hpp"

SimulationManager::Toolbar::Toolbar(QWidget* aParent)
   : QToolBar("Sim Execution", aParent)
   , mText("")
{
   setObjectName("SimulationManager");
   setIconSize(QSize(16, 16));
   mSimLabel = new QLabel();
   addWidget(mSimLabel);

   connect(mSimLabel, &QLabel::linkActivated, this, &Toolbar::LinkClicked);
   connect(qGuiApp, &QGuiApplication::paletteChanged, this, &Toolbar::PaletteChanged);
}

void SimulationManager::Toolbar::SetLabel(const QString& aText, const QString& aTextColor)
{
   mText = aText;
   if (aText.isEmpty())
   {
      mSimLabel->setText("<a href=\"foo://bar\"><b><font color = \"#FF0000\">No Sim Configured</font></b></a>");
   }
   else
   {
      QString tcolor = aTextColor;
      if (tcolor.isEmpty())
      {
         tcolor = palette().text().color().name(QColor::HexRgb);
      }
      mSimLabel->setText(QString("<a href=\"foo://bar\"><font color = \"" + tcolor + "\">") + aText + "</font></a>");
   }
}

void SimulationManager::Toolbar::PaletteChanged(const QPalette& aPalette)
{
   QString tcolor = aPalette.text().color().name(QColor::HexRgb);
   SetLabel(mText, tcolor);
}

void SimulationManager::Toolbar::LinkClicked()
{
   emit LinkActivated();
}
