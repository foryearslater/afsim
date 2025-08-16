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

#include "DialogBuilderResultsDialog.hpp"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QMoveEvent>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>

#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

static bool PointIsValid(QPoint aPoint, QSize aSize)
{
   for (const auto* screen : QGuiApplication::screens())
   {
      if (screen->geometry().contains(QRect(aPoint, aSize)))
      {
         return true;
      }
   }

   return false;
}

// static
WkDialogBuilder::ResultsDialog& WkDialogBuilder::ResultsDialog::MakeResultsDialog(QWidget*       aParent,
                                                                                  const QString& aScriptName,
                                                                                  const QString& aPlatformName)
{
   return *new ResultsDialog(aParent, aScriptName, aPlatformName);
}

WkDialogBuilder::ResultsDialog::ResultsDialog(QWidget* aParent, const QString& aScriptName, const QString& aPlatformName)
   : QDialog(aParent)
{
   Initialize(aScriptName, aPlatformName);
}

void WkDialogBuilder::ResultsDialog::AddReturnValue(const QString& aValue)
{
   mLayout->addWidget(new QLabel(aValue));
   mLayout->addStretch();
   resize(sizeHint());
}

void WkDialogBuilder::ResultsDialog::SetPosition(QPoint* aPoint, bool* aValid)
{
   mPoint = aPoint;

   // Assume true if aValid is null
   bool wasValid = (aValid == nullptr) || (*aValid);
   if (aValid != nullptr)
   {
      *aValid = (aPoint != nullptr);
   }

   if (aPoint != nullptr)
   {
      if (!wasValid || !PointIsValid(*aPoint, size()))
      {
         *aPoint = DefaultPoint();
      }
      // .move() does not give desired result.
      setGeometry(QRect(*aPoint, frameSize()));
   }
}

void WkDialogBuilder::ResultsDialog::moveEvent(QMoveEvent* aEvent)
{
   if (mPoint != nullptr)
   {
      *mPoint = aEvent->pos();
   }
}

void WkDialogBuilder::ResultsDialog::Initialize(const QString& aScriptName, const QString& aPlatformName)
{
   setAttribute(Qt::WA_DeleteOnClose,
                true); // This is how the dialog along with all other "newed" QWidget in this function are cleaned up
   auto* vlayout = new QVBoxLayout();
   setLayout(vlayout);
   setWindowTitle("Script Returns");

   auto glayout = new QGridLayout();
   vlayout->addLayout(glayout);

   glayout->addWidget(new QLabel("Script   Name:"), 0, 0);
   glayout->addWidget(new QLabel(aScriptName), 0, 1);

   if (!aPlatformName.isEmpty())
   {
      glayout->addWidget(new QLabel("Platform Name:"), 1, 0);
      glayout->addWidget(new QLabel(aPlatformName), 1, 1);
   }

   mLayout = new QVBoxLayout();

   glayout->addWidget(new QLabel("Return Value:"), 2, 0);
   glayout->addLayout(mLayout, 2, 1);

   auto* buttonBox = new QDialogButtonBox(this);
   buttonBox->setObjectName("buttonBox");
   buttonBox->setOrientation(Qt::Horizontal);
   buttonBox->setStandardButtons(QDialogButtonBox::Ok);

   connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::close);

   vlayout->addWidget(buttonBox);
}

QPoint WkDialogBuilder::ResultsDialog::DefaultPoint() const
{
   const auto* screen = wkfEnv.GetMainWindow();
   if (screen != nullptr)
   {
      return QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), screen->geometry()).topLeft();
   }

   return QPoint(0, 0);
}
