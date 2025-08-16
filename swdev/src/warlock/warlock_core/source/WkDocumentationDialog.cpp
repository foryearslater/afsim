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
#include "WkDocumentationDialog.hpp"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>

#include "WkfEnvironment.hpp"

warlock::DocumentationDialog::DocumentationDialog(wkf::MainWindow* parent, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
   : wkf::DocumentationDialog(parent, f)
{
}

void warlock::DocumentationDialog::PopulateDialog()
{
   QFileInfo fi(wkfEnv.GetDocumentationDir());

   QHBoxLayout* hLayout = new QHBoxLayout();
   QVBoxLayout* vLayout = new QVBoxLayout();
   hLayout->addLayout(vLayout);

   QLabel* image = new QLabel(this);
   image->setMaximumSize(QSize(128, 128));
   image->setPixmap(QPixmap(QString::fromUtf8(":/images/Splash_AFSIM-War_BW_Sm.png")));
   image->setScaledContents(true);
   hLayout->addWidget(image);

   if (fi.isDir())
   {
      QString dir(fi.absoluteFilePath().append("/html/docs/"));
      CreateDocLink(dir + "warlock.html", "Main Page", vLayout);
      CreateDocLink(dir + "warlock_users_guide.html", "User's Guide", vLayout);
      CreateDocLink(dir + "warlock_reference_guide.html", "Reference Guide", vLayout);
      CreateDocLink(dir + "warlock_change_log.html", "Change Log", vLayout);
      CreateDocLink(dir + "main_page.html", "AFSIM Main Page", vLayout);
   }

   if (vLayout->count() == 0)
   {
      QLabel* labelPtr = new QLabel("<p style=\"font-size:10pt;\"><i>No documentation found</i></p>", this);
      vLayout->addWidget(labelPtr);
      vLayout->addStretch();
   }

   GetLayout()->addLayout(hLayout);

   QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
   buttonBox->setObjectName(QStringLiteral("buttonBox"));
   buttonBox->setOrientation(Qt::Horizontal);
   buttonBox->setStandardButtons(QDialogButtonBox::Ok);
   connect(buttonBox, &QDialogButtonBox::accepted, this, [this] { hide(); });
   GetLayout()->addWidget(buttonBox);
}
