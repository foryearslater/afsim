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

#ifndef CREATEPLATFORMDATACONTAINER_HPP
#define CREATEPLATFORMDATACONTAINER_HPP

#include <string>
#include <vector>

#include <QObject>

namespace WkCreatePlatform
{
class DataContainer : public QObject
{
   Q_OBJECT
public:
   DataContainer() {}
   DataContainer(const std::vector<std::string>& aPlatformTypesList)
      : mPlatformTypesList(aPlatformTypesList)
   {
   }

   void SetTypeList(const std::vector<std::string>& aTypeList)
   {
      mPlatformTypesList = aTypeList;
      emit TypeListChanged(mPlatformTypesList);
   }
   void                           Clear() { mPlatformTypesList.clear(); }
   const std::vector<std::string> GetPlatformTypeList() const { return mPlatformTypesList; }

signals:
   void TypeListChanged(const std::vector<std::string>& aTypeList);

private:
   std::vector<std::string> mPlatformTypesList;
};
} // namespace WkCreatePlatform

#endif