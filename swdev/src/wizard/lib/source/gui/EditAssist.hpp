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

#ifndef EDITASSIST_HPP
#define EDITASSIST_HPP

#include <QDialog>
#include <QPoint>
#include <QString>
#include <QTextBrowser>

#include "ParseResults.hpp"
#include "UsCtx.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"
#include "WsfPProxyPath.hpp"

namespace wizard
{
class BrowserWidget;
class Editor;
class WsfScriptLookup;

class SizedTextBrowser : public QTextBrowser
{
public:
   explicit SizedTextBrowser(QWidget* p)
      : QTextBrowser(p)
   {
   }
   QSize sizeHint() const override
   {
      document()->adjustSize();
      QSize s = document()->size().toSize();
      s.rwidth() += 50;
      return s;
   }
   QSize minimumSizeHint() const override { return sizeHint(); }
};

class VI_EXPORT EditAssist : public QDialog
{
public:
   EditAssist();
   ~EditAssist() override = default;

   static EditAssist* MakeAssistant(Editor* aEditControlPtr, UtTextDocumentLocation aPos);

   void           SetAssistantData(Editor* aEditControlPtr, UtTextDocumentLocation aPos);
   void           SetAssistantData(QPoint aCursorPos);
   static QString BuildFileURL(UtTextDocumentLocation aLocation, QString aText = "");

   static QString BuildFileURL(const QString& aFilePath, size_t aPos = ut::npos, const QString& aLabel = QString());
   static QString BuildHRef(const QString& aUrl, const QString& aLabel);
   static void    ProcessUrl(const QUrl& aUrl);

protected:
   BrowserWidget* GetBrowser();
   void           DocLinkClicked(const QUrl& aUrl);
   void           AnchorClicked(const QUrl& aUrl);
   void           DocSizeChanged(const QSizeF& aNewSize);
   void           changeEvent(QEvent* e) override;
   void           resizeEvent(QResizeEvent* e) override;
   void           Resized();

   QPoint                 mCursorPos;
   QTextBrowser*          mTextBrowserPtr;
   Editor*                mEditPtr;
   UtTextDocumentLocation mPos;
   BrowserWidget*         mBrowserPtr;
};

class EditAssist_Include : public EditAssist
{
public:
   void SetIncludeData(ParseResults& aResults, WsfParseSourceInclude* aSourcePtr, int aEntryIndex, TextSource* aTargetPtr);

   WsfParseSourceInclude* mSourcePtr;
   TextSource*            mTargetPtr;
   int                    mEntryIndex;
};

class EditAssist_Type : public EditAssist
{
public:
   void SetTypeData(ParseResults& aResults, const ParseResults::TypeInformation& aTypeInfo);
};

class VI_EXPORT EditAssist_Type2 : public EditAssist
{
public:
   void SetTypeData(ParseResults& aResults, WsfPProxyPath aProxyPath);
};

class EditAssist_Script : public EditAssist
{
public:
   bool SetScriptData(WsfScriptLookup& aLookup, const std::string& aSymbolName);
   void SetScriptValRef(WsfScriptLookup& aLookup, const UsValRef& aValRef);
   void SetScriptType(WsfScriptLookup& aLookup, UsType* aTypePtr);

protected:
};
} // namespace wizard

#endif
