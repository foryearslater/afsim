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

#ifndef SMALLWEBVIEW_HPP
#define SMALLWEBVIEW_HPP

#include "BrowserWidget.hpp"

namespace wizard
{

class SmallWebView : public BrowserWidget //: public QWebEngineView
{
public:
   SmallWebView(QWidget* p);
   ~SmallWebView() override = default;

   QSize sizeHint() const override;
};

} // namespace wizard

#endif
