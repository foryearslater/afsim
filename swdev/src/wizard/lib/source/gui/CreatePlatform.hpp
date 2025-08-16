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

#ifndef CREATEPLATFORM_HPP
#define CREATEPLATFORM_HPP

#include <QAction>

#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"

namespace wizard
{

class VI_EXPORT CreatePlatform : public QAction
{
   Q_OBJECT
public:
   CreatePlatform(const WsfPProxyNode& aNode, QWidget* aParentWidget = nullptr, QObject* aParent = nullptr);
private slots:
   void OnTriggered();

private:
   WsfPProxyNode mNode;
   QWidget*      mParentWidgetPtr;
};

} // namespace wizard

#endif
