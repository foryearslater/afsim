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

#include "SmallWebView.hpp"

#include <QSize>

wizard::SmallWebView::SmallWebView(QWidget* p)
   : BrowserWidget(p)
{
}


QSize wizard::SmallWebView::sizeHint() const
{
   return QSize(400, 300);
}
