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

#include "ErrorDialog.hpp"

#include <QStyle>

#include "Debug.hpp"

namespace PatternVisualizer
{
ErrorDialog::ErrorDialog(QTextDocument* aErrorMessagePtr, const QString& aApplicationTitle, bool aIsFatal, QWidget* aParentPtr)
   : QDialog{aParentPtr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint}
{
   mUI.setupUi(this);
   setWindowTitle(aApplicationTitle + " Error");
   setWindowIcon(QIcon{":/images/pattern_visualizer.png"});
   mUI.errorText->setDocument(aErrorMessagePtr);

   QStyle* style  = QApplication::style();
   QPixmap pixmap = style->standardPixmap(QStyle::StandardPixmap::SP_MessageBoxCritical);
   mUI.criticalIconLabel->setPixmap(pixmap);
   mUI.criticalIconLabel->setFixedSize(pixmap.width(), pixmap.height());

   connect(mUI.quitButton, &QPushButton::released, [=]() { exit(EXIT_FAILURE); });
   connect(mUI.debugButton, &QPushButton::released, [=]() { Debug::DebuggerBreak(); });
   connect(mUI.continueButton, &QPushButton::released, [=]() { close(); });

   if (aIsFatal)
   {
      connect(this, &QDialog::finished, this, [=](int) { exit(EXIT_FAILURE); });
      mUI.continueButton->setVisible(false);
      mUI.headingLabel->setText("A fatal error has occurred");
   }
}
} // namespace PatternVisualizer
