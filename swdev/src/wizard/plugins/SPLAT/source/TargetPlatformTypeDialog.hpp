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

#ifndef TARGETPLATFORMTYPEDIALOG_HPP
#define TARGETPLATFORMTYPEDIALOG_HPP

#include <QDialog>
#include <QValidator>

#include "WsfPM_ObjectMap.hpp"
#include "ui_TargetPlatformTypeDialog.h"

namespace SPLAT
{
class TargetTypeValidator;
class SignatureValidator;

class TargetPlatformTypeDialog : public QDialog
{
   Q_OBJECT
public:
   struct TargetType
   {
      TargetType()
         : mPlatformType()
         , mRadarSigType()
         , mInfraredSigType()
         , mOpticalSigType()
         , mAcousticSigType()
         , mPlatformValue()
         , mRadarSigValue("1")
         , mInfraredSigValue("1")
         , mOpticalSigValue("1")
         , mAcousticSigValue("0")
      {
      }

      QString mPlatformType;
      QString mRadarSigType;
      QString mInfraredSigType;
      QString mOpticalSigType;
      QString mAcousticSigType;

      QString mPlatformValue;
      QString mRadarSigValue;
      QString mInfraredSigValue;
      QString mOpticalSigValue;
      QString mAcousticSigValue;
   };

   TargetPlatformTypeDialog(QWidget* aParent);
   ~TargetPlatformTypeDialog() override;

   void RestoreAllDefaults();

   void    WriteData(std::ofstream& aStream);
   QString GetTargetPlatformType() { return mTargetType.mPlatformType; }
   QString GetRadarSignatureType() { return mTargetType.mRadarSigType; }
   QString GetInfraredSignatureType() { return mTargetType.mInfraredSigType; }
   QString GetOpticalSignatureType() { return mTargetType.mOpticalSigType; }
   QString GetAcousticSignatureType() { return mTargetType.mAcousticSigType; }
   QString GetRadarSignatureValue() { return mTargetType.mRadarSigType; }
   QString GetInfraredSignatureValue() { return mTargetType.mInfraredSigType; }
   QString GetOpticalSignatureValue() { return mTargetType.mOpticalSigType; }
   QString GetAcousticSignatureValue() { return mTargetType.mAcousticSigType; }

   void UpdatePlatformTypes(const QStringList& aPlatformTypes);
   void UpdateRadarSignatures(const QStringList& aRadarSignatures);
   void UpdateInfraredSignatures(const QStringList& aInfraredSignatures);
   void UpdateOpticalSignatures(const QStringList& aOpticalSignatures);
   void UpdateAcousticSignatures(const QStringList& aAcousticSignatures);

   void CheckValidity();

private:
   void accept() override;
   void Connect();
   void TextChangedHandler();
   bool TargetTypeValid();
   void SetTargetType();

   Ui::TargetPlatformTypeDialog mUi;

   TargetTypeValidator* mTargetTypeValidator;
   SignatureValidator*  mRadarValidator;
   SignatureValidator*  mInfraredValidator;
   SignatureValidator*  mOpticalValidator;
   SignatureValidator*  mAcousticValidator;

   TargetType mTargetType;
   bool       mSignaturesEnabled;
};

class TargetTypeValidator : public QValidator
{
   Q_OBJECT
public:
   TargetTypeValidator(QObject* aParent = nullptr)
      : QValidator(aParent)
      , mList(QStringList())
      , mState(QValidator::Acceptable)
   {
   }

   void setValidList(const QStringList& aList) { mList = aList; }

   QValidator::State validate(QString& s, int& i) const override
   {
      QValidator::State valid = QValidator::Acceptable;
      if (!s.compare("TARGET_PLATFORM_TYPE") || (!s.isEmpty() && !mList.contains(s)))
      {
         valid = QValidator::Intermediate;
      }
      return valid;
   }

private:
   QStringList       mList;
   QValidator::State mState;
};

class SignatureValidator : public QValidator
{
   Q_OBJECT
public:
   SignatureValidator(QObject* aParent = nullptr)
      : QValidator(aParent)
      , mList(QStringList())
      , mState(QValidator::Acceptable)
   {
   }

   void setValidList(const QStringList& aList) { mList = aList; }

   QValidator::State validate(QString& s, int& i) const override
   {
      QValidator::State valid = QValidator::Acceptable;
      if (!s.isEmpty() && !mList.contains(s))
      {
         valid = QValidator::Intermediate;
      }
      return valid;
   }

private:
   QStringList       mList;
   QValidator::State mState;
};
} // namespace SPLAT
#endif // !TARGETPLATFORMTYPEDIALOG_HPP
