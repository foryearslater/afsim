// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "OutputVariablesWidget.hpp"

#include <ostream>

#include <QLineEdit>

SPLAT::OutputVariablesWidget::OutputVariablesWidget(PlotOptionsDialog* aParent)
   : PlotOptionsWidget(aParent)
{
   mUi.setupUi(this);
   Initialize();
   Connect();
}

void SPLAT::OutputVariablesWidget::WriteData(std::ostream& aStream)
{
   for (auto&& s : mSelectedVariables)
   {
      aStream << "   variable " << s.toStdString() << "\n";
   }
   aStream << "\n";
}

bool SPLAT::OutputVariablesWidget::ApplyChanges()
{
   SetVariables();
   return true;
}

void SPLAT::OutputVariablesWidget::RestoreDefaults()
{
   QList<QObject*> varList = mUi.mVariablesGroupBox->children();
   for (auto v : varList)
   {
      QCheckBox* checkBox = dynamic_cast<QCheckBox*>(v);
      if (checkBox)
      {
         checkBox->setChecked(false);
      }
   }
   mSelectedVariables.clear();
   Initialize();
}

void SPLAT::OutputVariablesWidget::Initialize()
{
   mUi.mPDCheckBox->setChecked(true);
   mSelectedVariables.push_back("pd");
}

void SPLAT::OutputVariablesWidget::Connect()
{
   connect(mUi.mRJPCheckBox,
           &QCheckBox::toggled,
           this,
           [this](bool aChecked) { emit RequiredJammingPowerPressed(aChecked); });
}

void SPLAT::OutputVariablesWidget::SetVariables()
{
   mSelectedVariables.clear();
   QList<QObject*> varList = mUi.mVariablesGroupBox->children();

   for (auto v : varList)
   {
      QCheckBox* checkBox = dynamic_cast<QCheckBox*>(v);
      if (checkBox)
      {
         if (checkBox->isChecked())
         {
            mSelectedVariables.push_back(checkBox->text());
         }
      }
   }
}
