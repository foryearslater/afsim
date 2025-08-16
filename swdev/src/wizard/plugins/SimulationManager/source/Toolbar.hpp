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

#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include <QLabel>
#include <QToolBar>

namespace SimulationManager
{
class Toolbar : public QToolBar
{
   Q_OBJECT
public:
   Toolbar(QWidget* aParent);
   void SetLabel(const QString& aText, const QString& aTextColor = "");
signals:
   void LinkActivated();

private:
   void    PaletteChanged(const QPalette& aPalette);
   void    LinkClicked();
   QLabel* mSimLabel;
   QString mText;
};
} // namespace SimulationManager

#endif
