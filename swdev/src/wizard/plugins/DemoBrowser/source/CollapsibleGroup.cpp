// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CollapsibleGroup.hpp"

DemoBrowser::CollapsibleGroup::CollapsibleGroup(QWidget* aParent, const QString& aTitle)
   : QGroupBox(aParent)
{
   setLayout(new QVBoxLayout(this));
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
   mHeaderPtr = new QWidget(this);
   layout()->addWidget(mHeaderPtr);
   mContentPtr = new QWidget(this);
   layout()->addWidget(mContentPtr);
   mHeaderPtr->setLayout(new QHBoxLayout(mHeaderPtr));
   mContentPtr->setLayout(new QVBoxLayout(mContentPtr));
   mTitlePtr = new QLabel(QString("<h3><b>") + aTitle + QString("</b></h3>"), mHeaderPtr);
   mHeaderPtr->layout()->addWidget(mTitlePtr);
   mHeaderPtr->layout()->setAlignment(Qt::AlignLeft);

   mHeaderPtr->layout()->setContentsMargins(0, 0, 0, 0);
   mContentPtr->layout()->setContentsMargins(0, 0, 0, 0);

   SetExpanded(true);
}
