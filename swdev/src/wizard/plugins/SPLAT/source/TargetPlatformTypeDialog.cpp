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

#include "TargetPlatformTypeDialog.hpp"

#include <QMessageBox>
#include <QPushButton>

#include "UtQtText.hpp"

SPLAT::TargetPlatformTypeDialog::TargetPlatformTypeDialog(QWidget* aParent)
   : QDialog(aParent)
   , mTargetTypeValidator(new TargetTypeValidator)
   , mRadarValidator(new SignatureValidator)
   , mInfraredValidator(new SignatureValidator)
   , mOpticalValidator(new SignatureValidator)
   , mAcousticValidator(new SignatureValidator)
   , mSignaturesEnabled(true)
{
   setWindowTitle("Target Platform Type");
   mUi.setupUi(this);

   mUi.mPlatformTypeEdit->setValidator(mTargetTypeValidator);
   mUi.mRadarSigEdit->setValidator(mRadarValidator);
   mUi.mInfraredSigEdit->setValidator(mInfraredValidator);
   mUi.mOpticalSigEdit->setValidator(mOpticalValidator);
   mUi.mAcousticSigEdit->setValidator(mAcousticValidator);

   Connect();
}

SPLAT::TargetPlatformTypeDialog::~TargetPlatformTypeDialog()
{
   delete mTargetTypeValidator;
   delete mRadarValidator;
   delete mInfraredValidator;
   delete mOpticalValidator;
   delete mAcousticValidator;
}

void SPLAT::TargetPlatformTypeDialog::RestoreAllDefaults()
{
   mTargetType = TargetType();
   mUi.mTargetPlatformTypeButton->setChecked(true);
   mUi.mPlatformTypeEdit->setText("");
   mUi.mRadarSigEdit->setText("");
   mUi.mInfraredSigEdit->setText("");
   mUi.mOpticalSigEdit->setText("");
   mUi.mAcousticSigEdit->setText("");
   mUi.mRadarSigValueEdit->setText("1");
   mUi.mInfraredSigValueEdit->setText("1");
   mUi.mOpticalSigValueEdit->setText("1");
   mUi.mAcousticSigValueEdit->setText("0.0");
}

void SPLAT::TargetPlatformTypeDialog::WriteData(std::ofstream& aStream)
{
   if (aStream.good())
   {
      // only write the data if the user hasn't specified a platformType that is already defined.
      if (mUi.mTargetPlatformTypeButton->isChecked())
      {
         // TODO make mutually exclusive with defined radar sig string
         if (mTargetType.mRadarSigType.isEmpty())
         {
            aStream << "\nradar_signature TARGET_RADAR_SIG\n   constant ";
            aStream << mTargetType.mRadarSigValue.toStdString();
            aStream << " m^2\nend_radar_signature\n";
         }

         if (mTargetType.mInfraredSigType.isEmpty())
         {
            aStream << "\ninfrared_signature TARGET_INFRARED_SIG\n   constant ";
            aStream << mTargetType.mInfraredSigValue.toStdString();
            aStream << " w/sr\nend_infrared_signature\n";
         }

         if (mTargetType.mOpticalSigType.isEmpty())
         {
            aStream << "\noptical_signature TARGET_OPTICAL_SIG\n   constant ";
            aStream << mTargetType.mOpticalSigValue.toStdString();
            aStream << " m^2\nend_optical_signature\n";
         }

         // if (mTargetType.mAcousticSig.isEmpty())
         //{
         //    aStream << "acoustic_signature TARGET_ACOUSTIC_SIG\n   constant "
         //            << mTargetType.mAcousticSigValue.toStdString()
         //            << " db\nend_acoustic_signature\n\n";
         // }

         aStream << "\nplatform_type TARGET_PLATFORM_TYPE WSF_PLATFORM\n";
         // if signature has been defined by user, write out
         std::string sigName = "TARGET_RADAR_SIG";
         if (!mTargetType.mRadarSigType.isEmpty())
         {
            sigName = mTargetType.mRadarSigType.toStdString();
         }
         aStream << "   radar_signature " << sigName << "\n";

         sigName = "TARGET_INFRARED_SIG";
         if (!mTargetType.mInfraredSigType.isEmpty())
         {
            sigName = mTargetType.mInfraredSigType.toStdString();
         }
         aStream << "   infrared_signature " << sigName << "\n";

         sigName = "TARGET_OPTICAL_SIG";
         if (!mTargetType.mOpticalSigType.isEmpty())
         {
            sigName = mTargetType.mOpticalSigType.toStdString();
         }
         aStream << "   optical_signature " << sigName << "\n";

         // sigName = "TARGET_ACOUSTIC_SIG";
         // if (!mTargetType.mAcousticSigType.isEmpty())
         //{
         //    sigName = mTargetType.mAcousticSigType.toStdString();
         // }
         // aStream << "   acoustic_signature " << sigName << "\n";

         aStream << "end_platform_type\n";
      }
   }
}

void SPLAT::TargetPlatformTypeDialog::UpdatePlatformTypes(const QStringList& aPlatformTypes)
{
   // if TARGET_PLATFORM_TYPE is defined in the proxy, we will use the definition already defined.
   // Specifying the signatures won't do anything, so disable those widgets
   mSignaturesEnabled = !aPlatformTypes.contains("TARGET_PLATFORM_TYPE");
   mTargetTypeValidator->setValidList(aPlatformTypes);
   mUi.mSignaturesGroupBox->setEnabled(mSignaturesEnabled && mUi.mTargetPlatformTypeButton->isChecked());
}

void SPLAT::TargetPlatformTypeDialog::UpdateRadarSignatures(const QStringList& aRadarSignatures)
{
   mRadarValidator->setValidList(aRadarSignatures);
}

void SPLAT::TargetPlatformTypeDialog::UpdateInfraredSignatures(const QStringList& aInfraredSignatures)
{
   mInfraredValidator->setValidList(aInfraredSignatures);
}

void SPLAT::TargetPlatformTypeDialog::UpdateOpticalSignatures(const QStringList& aOpticalSignatures)
{
   mOpticalValidator->setValidList(aOpticalSignatures);
}

void SPLAT::TargetPlatformTypeDialog::UpdateAcousticSignatures(const QStringList& aAcousticSignatures)
{
   mAcousticValidator->setValidList(aAcousticSignatures);
}

void SPLAT::TargetPlatformTypeDialog::accept()
{
   if (TargetTypeValid())
   {
      SetTargetType();
      close();
   }
   else
   {
      QMessageBox::critical(this, "Error", "Target Platform Type or Signature Invalid");
   }
}

void SPLAT::TargetPlatformTypeDialog::Connect()
{
   // on text changed, check state of validator and set text color accordingly
   connect(mUi.mPlatformTypeEdit, &QLineEdit::textChanged, this, &TargetPlatformTypeDialog::TextChangedHandler);
   connect(mUi.mRadarSigEdit, &QLineEdit::textChanged, this, &TargetPlatformTypeDialog::TextChangedHandler);
   connect(mUi.mInfraredSigEdit, &QLineEdit::textChanged, this, &TargetPlatformTypeDialog::TextChangedHandler);
   connect(mUi.mOpticalSigEdit, &QLineEdit::textChanged, this, &TargetPlatformTypeDialog::TextChangedHandler);
   connect(mUi.mAcousticSigEdit, &QLineEdit::textChanged, this, &TargetPlatformTypeDialog::TextChangedHandler);

   connect(mUi.mUserDefinedTypeButton,
           &QPushButton::toggled,
           this,
           [this](bool aPressed) { mUi.mPlatformTypeEdit->setEnabled(aPressed); });
   connect(mUi.mTargetPlatformTypeButton,
           &QPushButton::toggled,
           this,
           [this](bool aPressed) { mUi.mSignaturesGroupBox->setEnabled(aPressed && mSignaturesEnabled); });
}

void SPLAT::TargetPlatformTypeDialog::TextChangedHandler()
{
   QLineEdit* edit = (QLineEdit*)sender();
   UtQtText::SetValid(edit->hasAcceptableInput(), *edit);
}

void SPLAT::TargetPlatformTypeDialog::CheckValidity()
{
   UtQtText::SetValid(mUi.mPlatformTypeEdit->hasAcceptableInput(), *mUi.mPlatformTypeEdit);
   UtQtText::SetValid(mUi.mRadarSigEdit->hasAcceptableInput(), *mUi.mRadarSigEdit);
   UtQtText::SetValid(mUi.mInfraredSigEdit->hasAcceptableInput(), *mUi.mInfraredSigEdit);
   UtQtText::SetValid(mUi.mOpticalSigEdit->hasAcceptableInput(), *mUi.mOpticalSigEdit);
   UtQtText::SetValid(mUi.mAcousticSigEdit->hasAcceptableInput(), *mUi.mAcousticSigEdit);
}

bool SPLAT::TargetPlatformTypeDialog::TargetTypeValid()
{
   return ((mUi.mUserDefinedTypeButton->isChecked() && mUi.mPlatformTypeEdit->hasAcceptableInput() &&
            !mUi.mPlatformTypeEdit->text().isEmpty()) ||
           (mUi.mTargetPlatformTypeButton->isChecked() &&
            (mUi.mRadarSigEdit->hasAcceptableInput() || mUi.mRadarSigValueEdit->hasAcceptableInput()) &&
            (mUi.mInfraredSigEdit->hasAcceptableInput() || mUi.mInfraredSigValueEdit->hasAcceptableInput()) &&
            (mUi.mOpticalSigEdit->hasAcceptableInput() || mUi.mOpticalSigValueEdit->hasAcceptableInput()) &&
            (mUi.mAcousticSigEdit->hasAcceptableInput() || mUi.mAcousticSigValueEdit->hasAcceptableInput())));
}

void SPLAT::TargetPlatformTypeDialog::SetTargetType()
{
   mTargetType.mPlatformType     = mUi.mUserDefinedTypeButton->isChecked() ? mUi.mPlatformTypeEdit->text() : "";
   mTargetType.mRadarSigType     = mUi.mRadarSigEdit->text();
   mTargetType.mInfraredSigType  = mUi.mInfraredSigEdit->text();
   mTargetType.mOpticalSigType   = mUi.mOpticalSigEdit->text();
   mTargetType.mAcousticSigType  = mUi.mAcousticSigEdit->text();
   mTargetType.mRadarSigValue    = mUi.mRadarSigValueEdit->text();
   mTargetType.mInfraredSigValue = mUi.mInfraredSigValueEdit->text();
   mTargetType.mOpticalSigValue  = mUi.mOpticalSigValueEdit->text();
   mTargetType.mAcousticSigValue = mUi.mAcousticSigValueEdit->text();
}