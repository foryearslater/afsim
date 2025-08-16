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

#ifndef ASTROLABE_HPP
#define ASTROLABE_HPP

#include <memory>

class QMenu;
#include <QObject>
class QString;
class QWidget;

#include "UtCallbackHolder.hpp"

namespace wkf
{

class Entity;

}

namespace wizard
{

class Project;
class ProxyChange;

} // namespace wizard

#include "WsfPM_Platform.hpp"
class WsfPProxyNode;

#include "AstrolabeConfig.hpp"
#include "AstrolabeUI.hpp"

namespace SpaceTools
{

//! The Astrolabe tool.
class Astrolabe : public QObject
{
   Q_OBJECT

public:
   explicit Astrolabe();
   ~Astrolabe() override = default;

   void BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr);

public slots:
   void OnProjectOpened(wizard::Project* aProjectPtr);
   void OnProjectClosed(wizard::Project* aProjectPtr);
   void OnFullParseComplete(wizard::Project* aProjectPtr);
   void ShowToolForPlatform(const QString& aPlatformName);
   void LoadPlatform(const QString& aPlatformName);

private slots:
   void StartWatchingProxy();
   void StopWatchingProxy();
   void ProduceOutput();
   void Verify();
   void DockVisibilityChanged(bool aIsVisble);

private:
   void ActionsForNodeRequested(const WsfPProxyNode& aNode, QMenu* aMenuPtr, QWidget* aParentWidgetPtr);
   void OnProxyModified(const wizard::ProxyChange& aProxyChange);
   void PopulateSpacePlatforms();
   bool IsSpacePlatform(const QString& aPlatformName) const;

   wsfg::spaceg::AstrolabeConfig mConfig;
   AstrolabeUI*                  mAstrolabeUI_Ptr;
   wizard::Project*              mProjectPtr;
   UtCallbackHolder              mProxyModified;
};

} // namespace SpaceTools

#endif // ASTROLABE_HPP
