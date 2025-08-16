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

#ifndef VIEWTYPESUMMARYACTION_HPP
#define VIEWTYPESUMMARYACTION_HPP

#include <QAction>
#include <QPoint>

#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"

namespace wizard
{

class VI_EXPORT ViewTypeSummaryAction : public QAction
{
   Q_OBJECT
public:
   ViewTypeSummaryAction(const WsfPProxyNode& aNode, QObject* aParent = nullptr);
   ~ViewTypeSummaryAction() override = default;
private slots:
   void OnTriggered();

private:
   WsfPProxyNode mNode;
   QPoint        mPos;
};

} // namespace wizard


#endif
