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

#ifndef COMPONENTPROPERTYMANAGER_HPP
#define COMPONENTPROPERTYMANAGER_HPP


#include <map>
#include <string>

#include <QColor>
#include <QIcon>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>

#include "ViExport.hpp"
#include "WsfPProxyValue.hpp"

namespace wizard
{
class VI_EXPORT ComponentPropertyManager : public QObject
{
   Q_OBJECT
public:
   static void                      CreateInstance();
   static void                      DestroyInstance();
   static ComponentPropertyManager* Instance() { return sFactoryPtr; }
   ComponentPropertyManager();

   QString FindPartIcon(WsfPProxyValue aValue);

   std::map<std::string, QString> mPartIcons;
   std::map<QString, QColor>      mSideColors;
   std::map<QString, QString>     mSpatialDomainIcons;

   static ComponentPropertyManager* sFactoryPtr;
};
} // namespace wizard
#endif
