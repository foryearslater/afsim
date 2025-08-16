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

#include "BrowserWidget.hpp"

#include "DocumentationInterpretter.hpp"
#include "Environment.hpp"
#include "RunEnvManager.hpp"
#include "UtQt.hpp"

wizard::BrowserWidget::BrowserWidget(QWidget* aParentPtr)
   : QWidget(aParentPtr)
   , mDocumentName("")
{
   mUi.setupUi(this);

   mUi.webView->setOpenLinks(false);
   mUi.webView->setOpenExternalLinks(false);
   QObject::connect(mUi.webView, &QTextBrowser::anchorClicked, this, &BrowserWidget::LinkClicked);
}

void wizard::BrowserWidget::WebTitleChanged(const QString& title)
{
   setWindowTitle(title);
}

void wizard::BrowserWidget::LinkClicked(const QUrl& aUrl)
{
   QUrl lUrl = aUrl;
   if (lUrl.scheme().isEmpty())
   {
      if (aUrl.path().isEmpty())
      {
         lUrl = QString::fromStdString(
            RunEnvManager::DocumentationURL(mDocumentName.toStdString(), aUrl.fragment().toStdString()));
      }
      else
      {
         QString docPath = mDocumentName.left(mDocumentName.lastIndexOf('/'));
         lUrl =
            QString::fromStdString(RunEnvManager::DocumentationURL(docPath.toStdString() + "/" + aUrl.path().toStdString(),
                                                                   aUrl.fragment().toStdString()));
      }
      // reconstruct a relative link
   }
   // std::cout << "DEBUG: BrowserWidget::LinkClicked going to " << lUrl.toDisplayString().toStdString() << std::endl;
   wizEnv.OpenUrl(lUrl);
}

void wizard::BrowserWidget::SetAddress(const QUrl& aUrl)
{
   // std::cout << "DEBUG: BrowserWidget::SetAddress(" << aUrl.toDisplayString().toStdString() << ")!" << std::endl;
   bool    returl;
   QString content = DocumentationInterpretter::LookupDocumentationUrl(aUrl, returl, mDocumentName);
   if (returl)
   {
      // std::cout << "DEBUG: BrowserWidget::SetAddress redirect: " << content.toStdString() << std::endl;
      mUi.webView->setSource(QUrl(content));
   }
   else
   {
      // std::cout << "DEBUG: BrowserWidget::SetAddress html: " << content.toStdString() << std::endl;
      mUi.webView->setHtml(content);
   }
}

QTextBrowser* wizard::BrowserWidget::GetWebView()
{
   return mUi.webView;
}
