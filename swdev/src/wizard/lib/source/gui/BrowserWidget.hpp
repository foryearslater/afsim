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
#ifndef BROWSERWIDGET_HPP
#define BROWSERWIDGET_HPP

#include <QUrl>
#include <QWidget>

#include "UtMemory.hpp"
#include "ViExport.hpp"
#include "ui_BrowserWidget.h"

class QTextBrowser;

namespace wizard
{
class VI_EXPORT BrowserWidget : public QWidget
{
public:
   BrowserWidget(QWidget* aParentPtr = nullptr);
   BrowserWidget(const BrowserWidget&) = delete;
   BrowserWidget(BrowserWidget&&)      = default;
   BrowserWidget& operator=(const BrowserWidget&) = delete;
   BrowserWidget& operator=(BrowserWidget&&) = default;
   ~BrowserWidget() override                 = default;

   virtual void WebTitleChanged(const QString& title);
   virtual void SetAddress(const QUrl& aUrl);

   QTextBrowser* GetWebView();

protected:
   void LinkClicked(const QUrl& aUrl);

   Ui::BrowserWidget mUi;
   QString           mDocumentName;
};
} // namespace wizard
#endif
