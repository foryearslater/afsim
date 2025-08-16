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

#include "EditAssist.hpp"

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QLayout>
#include <QMessageBox>
#include <QUrl>
#include <QUrlQuery>

#include "Editor.hpp"
#include "Environment.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "SmallWebView.hpp"
#include "TextSource.hpp"
#include "UiResources.hpp"
#include "UtPath.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfExe.hpp"
#include "WsfPProxyObjectMap.hpp"
#include "WsfParseSourceInclude.hpp"
#include "WsfScriptLookup.hpp"

void wizard::EditAssist::SetAssistantData(QPoint aCursorPos)
{
   mCursorPos = aCursorPos;
}

QString wizard::EditAssist::BuildFileURL(UtTextDocumentLocation aLocation, QString aText /*= ""*/)
{
   if (aLocation.mSource)
   {
      UtPath  projDir = (static_cast<TextSource*>(aLocation.mSource))->GetProject()->ProjectDirectory();
      QString text    = aText;
      if (text.isEmpty())
      {
         std::string relpath = projDir.GetRelativePath(aLocation.mSource->GetFilePath()).GetSystemPath();
         text                = QString::fromStdString(relpath);
      }
      return BuildFileURL(QString::fromStdString(aLocation.mSource->GetFilePath().GetSystemPath()), aLocation.mPosition, text);
   }
   return "";
}

QString wizard::EditAssist::BuildFileURL(const QString& aFilePath,
                                         size_t         aPos /*= ut::npos*/,
                                         const QString& aLabel /*=QString()*/)
{
   QString lbl = aLabel;
   if (lbl.isEmpty())
   {
      lbl = aFilePath;
   }
   QUrl url = QUrl::fromLocalFile(aFilePath);
   if (aPos != ut::npos)
   {
      QUrlQuery query(url);
      query.addQueryItem("pos", QString::number(aPos));
      url.setQuery(query);
   }
   return BuildHRef(url.toString(), lbl);
}

QString wizard::EditAssist::BuildHRef(const QString& aUrl, const QString& aLabel)
{
   return Util::BuildHRef(aUrl, aLabel);
}

void wizard::EditAssist::ProcessUrl(const QUrl& aUrl)
{
   if (aUrl.scheme() == "file")
   {
      if (wizard::ProjectWorkspace::Instance()->GetProject())
      {
         if (QFile::exists(aUrl.toLocalFile()))
         {
            QUrlQuery query(aUrl);
            if (!query.hasQueryItem("pos") && !query.hasQueryItem("line"))
            {
               // if the offset isn't given, don't force opening in the text editor
               wizard::Project::Instance()->OpenFile(aUrl.toLocalFile().toStdString());
            }
            else
            {
               TextSource* sourcePtr =
                  wizard::ProjectWorkspace::Instance()->FindSource(aUrl.toLocalFile().toStdString(), true, false);
               if (sourcePtr)
               {
                  if (query.hasQueryItem("pos"))
                  {
                     int pos = query.queryItemValue("pos").toInt();
                     wizard::ProjectWorkspace::Instance()->GetProject()->GotoRange(UtTextDocumentRange(sourcePtr, pos, pos));
                  }
                  else if (query.hasQueryItem("line"))
                  {
                     size_t line = ut::cast_to_size_t(query.queryItemValue("line").toInt());
                     size_t col  = 0;
                     if (query.hasQueryItem("col"))
                     {
                        col = ut::cast_to_size_t(query.queryItemValue("col").toInt());
                     }
                     size_t pos = sourcePtr->GetLinePosition(line) + col;
                     wizard::ProjectWorkspace::Instance()->GetProject()->GotoRange(UtTextDocumentRange(sourcePtr, pos, pos));
                  }
                  else
                  {
                     wizard::ProjectWorkspace::Instance()->GetProject()->GotoFile(sourcePtr);
                  }
               }
            }
         }
         else
         {
            QMessageBox::information(wkfEnv.GetMainWindow(),
                                     "File Does Not Exist",
                                     QString("The file: ") + aUrl.toLocalFile() +
                                        " cannot be opened because it does not exist.");
         }
      }
   }
   else
   {
      if (aUrl.scheme() == "ide")
      {
         if (aUrl.host() == "documentation")
         {
            wizEnv.OpenUrl(aUrl);
         }
         else
         {
            emit wizSignals->URL_Process(aUrl);
         }
      }
   }
}

wizard::BrowserWidget* wizard::EditAssist::GetBrowser()
{
   if (mBrowserPtr)
   {
      return mBrowserPtr;
   }
   mBrowserPtr = new wizard::SmallWebView(this);
   ((QBoxLayout*)layout())->insertWidget(1, mBrowserPtr);
   connect(mBrowserPtr->GetWebView(), &QTextBrowser::anchorClicked, this, &EditAssist::DocLinkClicked);
   return mBrowserPtr;
}

void wizard::EditAssist::DocLinkClicked(const QUrl& aUrl)
{
   deleteLater();
}

void wizard::EditAssist::AnchorClicked(const QUrl& aUrl)
{
   EditAssist::ProcessUrl(aUrl);
   close();
}

void wizard::EditAssist::DocSizeChanged(const QSizeF& aNewSize)
{
   mTextBrowserPtr->setMinimumSize(aNewSize.toSize());
}

void wizard::EditAssist::changeEvent(QEvent* e)
{
   if (e->type() == QEvent::ActivationChange)
   {
      if (!isActiveWindow())
      {
         deleteLater();
      }
   }
}

void wizard::EditAssist::resizeEvent(QResizeEvent* e)
{
   QWidget::resizeEvent(e);
   QTimer::singleShot(1, this, &EditAssist::Resized);
}

void wizard::EditAssist::Resized()
{
   QPoint topLeft, bottomCenter;
   if (mEditPtr != nullptr)
   {
      size_t line, col;
      mEditPtr->GetLineColumn(mPos.mPosition, line, col);
      bottomCenter = mEditPtr->GetPoint(mPos.mPosition);
      bottomCenter = mEditPtr->mapToGlobal(bottomCenter);
   }
   else
   {
      bottomCenter = mCursorPos;
   }
   topLeft = bottomCenter;
   this->window()->height();
   topLeft.ry() -= window()->height() + 20;
   topLeft.rx() -= 300;
   QPoint bottomRight = topLeft;
   bottomRight.rx() += window()->width();
   bottomRight.ry() += window()->height();
   QDesktopWidget* desktopPtr = QApplication::desktop();

   int bestScreen = 0;
   for (int i = 0; i < desktopPtr->screenCount(); ++i)
   {
      QRect screenRect = desktopPtr->screenGeometry(i);
      if (screenRect.contains(bottomCenter))
      {
         bestScreen = i;
      }
   }

   QRect screenRect = desktopPtr->screenGeometry(bestScreen);
   int   diff       = screenRect.right() - bottomRight.x();
   if (diff < 0)
   {
      topLeft.rx() += diff;
   }
   diff = screenRect.bottom() - bottomRight.y();
   if (diff < 0)
   {
      topLeft.rx() += diff;
   }
   if (topLeft.x() < screenRect.left())
   {
      topLeft.setX(screenRect.left());
   }
   if (topLeft.y() < screenRect.top())
   {
      topLeft.setY(screenRect.top());
   }

   if (pos() != topLeft)
   {
      window()->move(topLeft);
   }
}

wizard::EditAssist::EditAssist()
   : QDialog(nullptr, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint)
   , mEditPtr(nullptr)
   , mBrowserPtr(nullptr)
{
   QVBoxLayout* layoutPtr = new QVBoxLayout;
   layoutPtr->setContentsMargins(0, 0, 0, 0);
   setLayout(layoutPtr);

   mTextBrowserPtr = new SizedTextBrowser(this);
   mTextBrowserPtr->setFont(UiResources::GetInstance().GetFont());

   layout()->addWidget(mTextBrowserPtr);
   mTextBrowserPtr->setOpenLinks(false);
   connect(mTextBrowserPtr, &QTextBrowser::anchorClicked, this, &EditAssist::AnchorClicked);
   connect(mTextBrowserPtr->document()->documentLayout(),
           &QAbstractTextDocumentLayout::documentSizeChanged,
           this,
           &EditAssist::DocSizeChanged);
}

wizard::EditAssist* wizard::EditAssist::MakeAssistant(Editor* aEditControlPtr, UtTextDocumentLocation aPos)
{
   TextSource* sourcePtr = static_cast<TextSource*>(aPos.mSource);
   if (!sourcePtr)
   {
      return nullptr;
   }
   wizard::Project* projectPtr = aEditControlPtr->GetProject();
   if (!projectPtr || !projectPtr->GetParseResults())
   {
      return nullptr;
   }
   ParseResults&          parseResults = *projectPtr->GetParseResults();
   UtTextDocumentRange    range(aPos, 0);
   WsfParseSourceInclude* sourceIncludePtr;
   int                    entryIndex;
   TextSource*            targetSourcePtr = nullptr;
   if (parseResults.FindIncludeAtPoint(sourcePtr->GetSource(), range, sourceIncludePtr, entryIndex, targetSourcePtr))
   {
      EditAssist_Include* includeAssist = new EditAssist_Include();
      includeAssist->SetAssistantData(aEditControlPtr, aPos);
      includeAssist->SetIncludeData(parseResults, sourceIncludePtr, entryIndex, targetSourcePtr);
      return includeAssist;
   }
   ParseResults::TypeInformation typeInfo;
   if (parseResults.TypeInformationFromPosition(aPos, typeInfo))
   {
      WsfPProxyValue proxyRoot = parseResults.ProxyRoot();
      WsfPProxyPath  path      = WsfPProxyPath::FromTypePath(proxyRoot, typeInfo.mTypeKey);
      if (proxyRoot.Lookup(path))
      {
         EditAssist_Type2* typeAssist = new EditAssist_Type2;
         typeAssist->SetAssistantData(aEditControlPtr, aPos);
         typeAssist->SetTypeData(parseResults, path);
         return typeAssist;
      }
      else
      {
         EditAssist_Type* typeAssist = new EditAssist_Type();
         typeAssist->SetAssistantData(aEditControlPtr, aPos);
         typeAssist->SetTypeData(parseResults, typeInfo);
         return typeAssist;
      }
   }
   WsfParseNode* nodePtr = parseResults.FindLastParseNode(UtTextDocumentRange(aPos, 0));
   // If the location is in a script, try to get information about the symbol
   if (nodePtr != nullptr && (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK))
   {
      UtTextDocumentRange identRange = WsfScriptLookup::IdentifierRange(range.GetPosition());
      if (identRange.Valid())
      {
         std::string     identName = identRange.Text();
         WsfScriptLookup lookup(parseResults.mProjectPtr);
         lookup.SetLocation(range.GetPosition(), WsfScriptLookup::cSTOP_AT_LOCATION);
         if (lookup.Update() >= WsfScriptLookup::cSUCCESS)
         {
            EditAssist_Script* scriptAssistPtr = new EditAssist_Script();
            scriptAssistPtr->SetAssistantData(aEditControlPtr, aPos);
            if (scriptAssistPtr->SetScriptData(lookup, identName))
            {
               return scriptAssistPtr;
            }
            delete scriptAssistPtr;
         }
      }
   }

   return nullptr;
}

void wizard::EditAssist::SetAssistantData(Editor* aEditControlPtr, UtTextDocumentLocation aPos)
{
   mEditPtr = aEditControlPtr;
   mPos     = aPos;
   size_t line, col;
   mEditPtr->GetLineColumn(aPos.mPosition, line, col);
}

void wizard::EditAssist_Include::SetIncludeData(ParseResults&          aResults,
                                                WsfParseSourceInclude* aSourcePtr,
                                                int                    aEntryIndex,
                                                TextSource*            aTargetPtr)
{
   mSourcePtr                        = aSourcePtr;
   mTargetPtr                        = aTargetPtr;
   mEntryIndex                       = aEntryIndex;
   QTextCursor                   cur = mTextBrowserPtr->textCursor();
   QString                       text;
   WsfParseSourceInclude::Entry& entry = aSourcePtr->mEntries[aEntryIndex];
   if (entry.mTargetSource)
   {
      QString filePath = entry.mTargetSource->GetFilePath().GetSystemPath().c_str();
      text.append("Include to " + BuildFileURL(filePath, -1));
   }
   else
   {
      text.append("<span style=\"color:red;\">Include file not found.</span>");
   }
   text.append("<BR/>");
   text.append("Root Directory: " + QString::fromStdString(aResults.mProjectPtr->WorkingDirectory().GetSystemPath()));
   text.append("<BR/>");
   for (auto& inc : aResults.mIncludePaths)
   {
      text.append("file_path " + QString::fromStdString(inc.Get()));
      text.append("<BR/>");
   }
   cur.insertHtml(text);
}

void wizard::EditAssist_Type::SetTypeData(ParseResults& aResults, const ParseResults::TypeInformation& aTypeInfo)
{
   QString     text;
   QTextCursor cur = mTextBrowserPtr->textCursor();
   if (aTypeInfo.mFlags & ParseResults::TypeInformation::cTYPE_LOOKUP_FAIL)
   {
      WsfParseNode* nodePtr = aResults.FindCreateTypeNode(aTypeInfo);
      if (nodePtr && nodePtr->GetParent())
      {
         WsfParseNode* leaf = nodePtr->NextLeaf();
         if (leaf && leaf->mValue.mSource)
         {
            text.append("<span style=\"color:red;\">Type exists, but is not in include.</span><BR/>");
            QString filePath = leaf->mValue.mSource->GetFilePath().GetSystemPath().c_str();
            text.append("<b>Fix: Add include to</b> " + BuildFileURL(filePath, -1, filePath));
            text.append("<BR/>");
         }
      }
      else
      {
         QString kind = QString::fromStdString(aTypeInfo.mTypeKey[0].Get());
         QString type = QString::fromStdString(aTypeInfo.mTypeKey[1].Get());
         text.append("<span style=\"color:red;\">" + kind + " not found (" + type + ")</span><BR/>");
         text.append("<b>Fix: Change to a valid type.</b>");
         text.append("<BR/>");
      }
   }
   else if (aTypeInfo.mFlags & ParseResults::TypeInformation::cDUPLICATE_NAME)
   {
      QString kind = QString::fromStdString(aTypeInfo.mTypeKey[0].Get());
      QString type = QString::fromStdString(aTypeInfo.mTypeKey[1].Get());
      text.append("<span style=\"color:red;\">" + kind + " has duplicate name (" + type + ")</span><BR/>");
      text.append("<b>Fix: Use a different name.</b>");
      text.append("<BR/>");
   }
   ParseResults::TypeInformation base, derived;
   derived = aTypeInfo;

   for (int i = 0; i < 10; ++i)
   {
      if (i != 0)
      {
         text.append("<b>Inherits:</b> ");
      }
      else
      {
         text.append("<b>" + QString::fromStdString(derived.mTypeKey[0].Get()) + "</b> ");
      }
      std::string typeName = derived.mTypeKey[1].Get();
      text.append(typeName.c_str());
      if (derived.mFlags & ParseResults::TypeInformation::cBASIC_TYPE)
      {
         WsfExe* exePtr = aResults.mProjectPtr->GetExe();
         if (exePtr)
         {
            std::shared_ptr<GrammarDocumentation> doc = exePtr->GetGrammarDocumentation();
            if (!doc)
            {
               const std::map<std::string, std::string>& titleToLink = doc->mPageTitleToLink;
               if (titleToLink.find(typeName) != titleToLink.end())
               {
                  QUrl newUrl;
                  newUrl.setScheme("ide");
                  newUrl.setHost("documentation");
                  newUrl.setPath("/docs/" + QString::fromStdString(typeName) + ".html");
                  text.append(BuildHRef(newUrl.toString(), "[Documentation]"));
               }
            }
         }
         text.append(" (built-in)");
      }
      else
      {
         WsfParseNode* nodePtr = aResults.FindTypeDefinitionNode(derived);
         if (nodePtr)
         {
            QString path = nodePtr->mValue.mSource->GetFilePath().GetSystemPath().c_str();
            text.append(BuildFileURL(path, nodePtr->mValue.GetBegin(), "[Jump To]"));
         }
      }

      std::string doc;
      if (aResults.FindUserTypeDocumentation(derived, doc))
      {
         text.append("<BR/><span style=\"color:green; margin-left: 40px;\"> &nbsp;&nbsp; " +
                     QString::fromStdString(doc) + "</span><BR/>");
      }
      else
      {
         text.append("<BR/>");
      }
      if (!aResults.FindBaseType(derived, base))
      {
         break;
      }
      derived = base;
   }
   cur.insertHtml(text);
}

void wizard::EditAssist_Type2::SetTypeData(ParseResults& aResults, WsfPProxyPath aProxyPath)
{
   QString              text;
   QTextCursor          cur       = mTextBrowserPtr->textCursor();
   WsfPProxyPath        path      = aProxyPath;
   WsfPProxyStructValue proxyRoot = aResults.ProxyRoot();
   WsfPProxyStructValue basicRoot = aResults.ProxyRegistry()->GetBasicRoot();

   for (int i = 0; (i < 10) && (path.size() > 1); ++i)
   {
      bool                 isBasic = false;
      WsfPProxyStructValue val     = proxyRoot.Lookup(path);
      if (!val.IsValid())
      {
         isBasic = true;
         val     = basicRoot.Lookup(path);
      }
      if (!val.IsValid())
      {
         break;
      }
      std::string typeListName = path.ToString(proxyRoot, path.size() - 2);
      std::string typeName     = path.ToString(proxyRoot, path.size() - 1);

      if (i != 0)
      {
         text.append("<b>Inherits:</b> ");
      }
      else
      {
         text.append("<b>" + QString::fromStdString(typeListName) + "</b> ");
      }
      text.append(typeName.c_str());
      if (isBasic)
      {
         WsfExe* exePtr = aResults.mProjectPtr->GetExe();
         if (exePtr)
         {
            std::shared_ptr<GrammarDocumentation> doc = exePtr->GetGrammarDocumentation();
            if (doc)
            {
               const std::map<std::string, std::string>& titleToLink = doc->mPageTitleToLink;
               if (titleToLink.find(typeName) != titleToLink.end())
               {
                  QUrl newUrl;
                  newUrl.setScheme("ide");
                  newUrl.setHost("documentation");
                  newUrl.setPath("/docs/" + QString::fromStdString(typeName) + ".html");
                  text.append(BuildHRef(newUrl.toString(), "[Documentation]"));
               }
            }
         }
         text.append(" (built-in)");
      }
      else
      {
         std::vector<UtTextDocumentRange> ranges;
         aResults.FindModificationLocationsExtended(path, ranges);
         for (auto&& r : ranges)
         {
            QString path     = r.mSource->GetFilePath().GetSystemPath().c_str();
            QString linkText = QString::fromStdString(r.mSource->GetFilePath().GetFileName());
            linkText += ":" + QString::number(r.mSource->PositionToLineNumber(r.GetBegin()) + 1);
            text.append(BuildFileURL(path, r.GetBegin(), "[" + linkText + "]"));
         }
      }

      std::string doc;
      text.append("<BR/>");
      if (val.GetBase())
      {
         path = *val.GetBase();
      }
      else
      {
         break;
      }
   }

   std::vector<WsfPProxyPath> derivedTypes;
   aResults.FindDerivedTypes(aProxyPath, derivedTypes);
   text += "Derived Types: ";

   for (auto&& path : derivedTypes)
   {
      std::string         name     = path.Back().GetMapKey();
      UtTextDocumentRange defRange = aResults.FindDefinitionLocation(path);

      text += " ";
      if (defRange.Valid())
      {
         text += BuildFileURL(defRange.GetPosition(), QString::fromStdString(name));
      }
      else
      {
         text += "<b>";
         text += QString::fromStdString(name);
         text += "</b>";
      }
   }

   if (derivedTypes.empty())
   {
      text += "<i>None</i><BR/>";
   }

   WsfPProxyValue value = proxyRoot.Lookup(aProxyPath);
   if (value.GetType() && value.GetType()->mTypeName == "Platform")
   {
      WsfPProxyObjectMap* cmdChainMap = value["commandChains"].GetObjectMap();
      if (cmdChainMap && !cmdChainMap->GetValues().empty())
      {
         std::string platformName = aProxyPath.Back().GetMapKey();
         text += "<b>Command Chains: </b>";
         const auto& chainValues = cmdChainMap->GetValues();
         for (auto&& cv : chainValues)
         {
            QString chainName = QString::fromStdString(cv.first);
            text += BuildHRef("ide://show_command_chain/" + chainName + "/" + platformName.c_str(), chainName);
            text += " ";
         }
         text += "<BR/>";
      }
   }
   text += "<BR/>";

   cur.insertHtml(text);
}

bool wizard::EditAssist_Script::SetScriptData(WsfScriptLookup& aLookup, const std::string& aSymbolName)
{
   UsType* typePtr = aLookup.FindType(aSymbolName);
   if (typePtr)
   {
      SetScriptType(aLookup, typePtr);
      return true;
   }
   else
   {
      // Note: We are given a script lookup object which has parsed up to the current position.
      // This is adequate to find most symbols by name, however, if a symbol is defined
      // on the same line, we will miss its definition.  In this case, we just
      // do another lookup parsing the remainder of the line to capture the new variable definition.
      WsfScriptLookup::SymbolMap symbols;
      aLookup.AddMethodSymbols(symbols);
      aLookup.AddSymbolsFromThis(symbols);
      if (symbols.find(aSymbolName) == symbols.end())
      {
         // finally, to look for a symbol parse until the end of line.
         // note that this will invalidate values in 'symbols'
         symbols.clear();
         aLookup.SetLocation(aLookup.GetLocation(), WsfScriptLookup::cSTOP_AT_END_OF_STATEMENT);
         if (aLookup.Update() >= WsfScriptLookup::cSUCCESS)
         {
            aLookup.AddSymbolsInScope(symbols);
         }
      }
      if (symbols.find(aSymbolName) != symbols.end())
      {
         UsValRef valRef = symbols[aSymbolName];
         SetScriptValRef(aLookup, valRef);
         return true;
      }
   }
   return false;
}

void wizard::EditAssist_Script::SetScriptValRef(WsfScriptLookup& aLookup, const UsValRef& aValRef)
{
   QString description = QString::fromStdString(aLookup.GetScriptValDescription(aValRef));
   QString defLocationLink;
   if (aValRef.mFunctionNamePtr == nullptr)
   {
      UtTextDocumentRange range = aLookup.GetDefinitionLocation(aValRef.mSymbolPtr);
      if (range.Valid())
      {
         defLocationLink = "File: " + EditAssist::BuildFileURL(range.GetPosition());
         description += "<BR/>";
         description += defLocationLink;
      }
   }
   else
   {
      std::string pageName;
      std::string sectionName;
      std::string fnName = aValRef.mFunctionNamePtr;
      // std::string fnType = aValRef.mFunctionTypePtr->mName;
      if (aLookup.GetScriptData()->FindScriptMethodDocumentation(*mEditPtr->GetProject()->GetExe()->GetGrammarDocumentation(),
                                                                 aValRef.mFunctionTypePtr,
                                                                 aValRef.mFunctionNamePtr,
                                                                 pageName,
                                                                 sectionName))
      {
         BrowserWidget* docView = GetBrowser();
         QUrl url(QString::fromStdString("ide://documentation/" + pageName + "?doc_section=" + sectionName + "." + fnName));
         docView->SetAddress(url);
         docView->show();
      }
   }
   mTextBrowserPtr->setText(description);
   layout()->update();
}

void wizard::EditAssist_Script::SetScriptType(WsfScriptLookup& aLookup, UsType* aTypePtr)
{
   UsTypeList*       typesPtr = aLookup.GetScriptData()->mTypesPtr->GetConstCtx()->mTypes;
   std::stringstream ss;
   ss << "type <b>" << aTypePtr->mName << "</b>";
   for (int i = (int)aTypePtr->mInheritedTypes.size() - 1; i >= 0; --i)
   {
      ss << "<BR/><i>inherits</i> " << typesPtr->GetTypeName(aTypePtr->mInheritedTypes[i]);
   }
   QString     text     = ss.str().c_str();
   std::string pageName = aTypePtr->mName + ".html";
   text += "<BR/>";
   text += BuildHRef(QString::fromStdString("ide://documentation/docs/" + pageName), "[Documentation]");
   mTextBrowserPtr->setText(text);
   layout()->update();
}
