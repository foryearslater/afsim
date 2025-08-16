// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "FileSignature.hpp"

#include "UtPath.hpp"

wizard::FileSignature::FileSignature()
   : mPreviousFileSizeStat(size_t())
   , mPreviousFileDateStat(time_t())
{
}

wizard::FileSignature wizard::FileSignature::GetSignature(UtPath& aPath)
{
   UtPath::StatData statData;
   aPath.Stat(statData);
   FileSignature sig;
   if (statData.mStatType == UtPath::cFILE)
   {
      sig.mPreviousFileDateStat = statData.mModifiedDate;
      sig.mPreviousFileSizeStat = statData.mFileSizeBytes;
   }
   return sig;
}

int wizard::FileSignature::Update(const UtPath& aPath)
{
   UtPath::StatData statData;
   aPath.Stat(statData);
   if (statData.mStatType != UtPath::cFILE)
   {
      return cFILE_NOT_FOUND;
   }

   int result = cNO_CHANGE;
   if (mPreviousFileDateStat != statData.mModifiedDate || mPreviousFileSizeStat != statData.mFileSizeBytes)
   {
      mPreviousFileSizeStat = statData.mFileSizeBytes;
      mPreviousFileDateStat = statData.mModifiedDate;
      result                = cTIME_CHANGE;
      // FUTURE: code to hash files:
      //       QFile file(aPath.GetSystemPath());
      //       QByteArray data = file.readAll();
      //       QCryptographicHash hash(QCryptographicHash::Sha1);
      //       hash.addData(data);
      //       std::string newHash = QString(hash.result().toHex()).toStdString();
      //       if (newHash != mHash)
      //       {
      //           mHash = newHash;
      //           result = cDATA_CHANGE;
      //       }
   }
   return result;
}
