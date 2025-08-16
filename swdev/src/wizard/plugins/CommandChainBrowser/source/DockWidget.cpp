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

#include "DockWidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMenu>
#include <QTextBlock>
#include <QUndoStack>

#include "EditAssist.hpp"
#include "Editor.hpp"
#include "EditorHelpers.hpp"
#include "EditorPrefObject.hpp"
#include "FindReferencesResultDialog.hpp"
#include "ParseResults.hpp"
#include "Platform.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "UtMemory.hpp"
#include "VaEnvironment.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Root.hpp"

CommandChain::DockWidget::DockWidget(QWidget* aParent)
   : QDockWidget(aParent)
   , mProjectPtr(nullptr)
   , mCommandChainModelPtr(nullptr)
{
   connect(wizSignals, &wizard::Signals::URL_Process, this, &DockWidget::URL_Process);

   setObjectName("CommandChainBrowser");
   setWidget(new QWidget(this));
   mUi.setupUi(widget());
   mUi.commandChainView->mParentPtr = this;

   // Make view more compact
   setWindowTitle(widget()->windowTitle());

   mUi.filterTeam->setContentsMargins(8, 2, 2, 2);
   mUi.filterTeam->addItem(cALL_SIDES);
   mUi.filterCommandChain->setContentsMargins(8, 2, 2, 2);
   mUi.filterCommandChain->addItem(cALL_CHAINS);
   mCallbacks += mUi.filterTeam->BeforePopup.Connect(&DockWidget::PopulateTeams, this);

   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectOpened, this, &DockWidget::ProjectOpened);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &DockWidget::ProjectClosed);

   connect(mUi.commandChainView, &View::activated, this, &DockWidget::ItemActivated);
   connect(mUi.filterTeam,
           static_cast<void (LazyComboBox::*)(const QString&)>(&LazyComboBox::currentIndexChanged),
           this,
           &DockWidget::FilterTeamChanged);
   connect(mUi.filterCommandChain,
           static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
           this,
           &DockWidget::FilterCommandChainChanged);
   connect(mUi.clearFilterBn, &QToolButton::clicked, this, &DockWidget::ClearSearch);
}

void CommandChain::DockWidget::ProjectOpened(wizard::Project* aProjectPtr)
{
   SetProject(aProjectPtr);
}

void CommandChain::DockWidget::ProjectClosed(wizard::Project* aProjectPtr)
{
   if (mCommandChainModelPtr)
      mCommandChainModelPtr->ClearScenario();
   mProjectPtr = aProjectPtr;
   ClearSearch();
}

void CommandChain::DockWidget::URL_Process(const QUrl& aUrl)
{
   if (!mProjectPtr)
      return;

   if (aUrl.scheme() == "ide" && aUrl.host() == "show_command_chain")
   {
      show();
      raise();
      mUi.commandChainView->activateWindow();
      QStringList pathList = aUrl.path().split('/', QString::SkipEmptyParts);
      if (!pathList.empty())
      {
         mShowChainName = pathList[0];
         mShowPlatformName.clear();
         if (pathList.size() > 1)
         {
            mShowPlatformName = pathList[1];
         }
         UpdateSelection();
      }
   }
}

void CommandChain::DockWidget::PopulateTeams()
{
   if (mProjectPtr)
   {
      wizard::ParseResults* results = mProjectPtr->GetParseResults();
      if (results && results->Proxy())
      {
         WsfPM_Root            root(results->Proxy());
         WsfPM_PlatformMap     platforms = root.platforms();
         std::set<std::string> sides;
         if (platforms)
         {
            for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
            {
               std::string side = platforms.AtT(i).Side();
               if (!side.empty())
               {
                  sides.insert(side);
               }
            }

            for (int i = 1; i < mUi.filterTeam->count();)
            {
               QString side = mUi.filterTeam->itemText(i);
               if (sides.find(side.toStdString()) == sides.end())
               {
                  mUi.filterTeam->removeItem(i);
               }
               else
               {
                  sides.erase(side.toStdString());
                  ++i;
               }
            }
            for (auto&& it : sides)
            {
               mUi.filterTeam->addItem(it.c_str());
            }
         }
      }
   }
}

void CommandChain::DockWidget::PopulateCommandChains()
{
   if (!mCommandChainModelPtr)
      return;
   QSet<QString>                    allChains;
   Model::CommandChainMap           commandChains = mCommandChainModelPtr->GetCommandChainMap();
   Model::CommandChainMap::iterator ccIter        = commandChains.begin();
   for (; ccIter != commandChains.end(); ++ccIter)
   {
      allChains.insert(ccIter->first.c_str());
   }

   if (allChains == mCommandChainNames)
      return;
   // first element is 'all'
   for (int i = 1; i < mUi.filterCommandChain->count();)
   {
      QString chain = mUi.filterCommandChain->itemText(i);
      if (!allChains.contains(chain))
      {
         mUi.filterCommandChain->removeItem(i);
      }
      else
      {
         ++i;
      }
   }
   QSet<QString> newChains = allChains;
   newChains.subtract(mCommandChainNames);
   mUi.filterCommandChain->addItems(newChains.toList());
   mCommandChainNames = allChains;
}

void CommandChain::DockWidget::ItemActivated(const QModelIndex& aIndex)
{
   if (!mCommandChainModelPtr)
      return;
   Item* itemPtr = FindItem(aIndex);
   if (nullptr != itemPtr)
   {
      OpenLocation(itemPtr);
   }
}

void CommandChain::DockWidget::FilterTeamChanged(const QString& aTeamText)
{
   if (aTeamText == cALL_SIDES)
   {
      mUi.commandChainView->mSideRegEx = QRegExp(".*");
   }
   else
   {
      mUi.commandChainView->mSideRegEx = QRegExp(aTeamText, Qt::CaseSensitive, QRegExp::FixedString);
   }
   mUi.commandChainView->FilterUpdateLater();
}

void CommandChain::DockWidget::FilterCommandChainChanged(const QString& aCommandChain)
{
   if (aCommandChain == cALL_CHAINS)
   {
      mUi.commandChainView->mCommandChainRegEx = QRegExp(".*");
   }
   else
   {
      mUi.commandChainView->mCommandChainRegEx = QRegExp(aCommandChain, Qt::CaseSensitive, QRegExp::FixedString);
   }
   mUi.commandChainView->FilterUpdateLater();
}

void CommandChain::DockWidget::ClearSearch()
{
   // mUi.filterText->clearEditText();
   mUi.filterTeam->setCurrentIndex(0);
   mUi.filterCommandChain->setCurrentIndex(0);
}

void CommandChain::DockWidget::SetProject(wizard::Project* aProjectPtr)
{
   mProjectPtr = aProjectPtr;
   ClearSearch();
}

void CommandChain::DockWidget::UpdateSelection()
{
   if (mShowChainName.isEmpty() || !mCommandChainModelPtr)
      return;
   Item* itemPtr = nullptr;
   if (!mShowPlatformName.isEmpty())
   {
      itemPtr =
         mCommandChainModelPtr->FindPlatformModelItem(mShowChainName.toStdString(), mShowPlatformName.toStdString());
   }
   else
   {
      itemPtr = mCommandChainModelPtr->FindCommandChainRoot(mShowChainName.toStdString());
   }
   if (itemPtr)
   {
      QModelIndex index = mCommandChainModelPtr->GetRowIndex(*itemPtr);
      mUi.commandChainView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
      mUi.commandChainView->scrollTo(index);
   }

   mShowChainName.clear();
}

CommandChain::Item* CommandChain::DockWidget::FindItem(const QModelIndex& aIndex)
{
   return dynamic_cast<Item*>(mCommandChainModelPtr->FindItem(aIndex));
}

void CommandChain::DockWidget::ContextEvent(const QModelIndex& aIndex, QPoint aPos)
{
   Item* itemPtr = FindItem(aIndex);
   if (itemPtr && itemPtr->mIsPlatform && mProjectPtr)
   {
      QMenu popup;
      popup.addAction("Collapse All", [=]() { CollapseChildren(aIndex); });
      popup.addAction("Expand All", [=]() { ExpandChildren(aIndex); });
      popup.addSeparator();
      popup.addAction("Platform Info...", [=]() { ShowPlatformInfo(itemPtr->mPlatform, aPos); });
      popup.addAction("Add to new command chain", [=]() { AddToNewCommandChain(itemPtr->mName); });
      popup.exec(QCursor::pos());
   }
}

bool CommandChain::DockWidget::OpenLocation(Item* aItemPtr)
{
   if (!aItemPtr || !mProjectPtr || !mProjectPtr->TinyParseResults())
      return false;
   wizard::ParseResultsTiny* resultsPtr = mProjectPtr->TinyParseResults();
   WsfPProxyIndex*           indexPtr   = resultsPtr->ProxyIndex();

   bool opened = false;

   if (indexPtr)
   {
      WsfPProxyNode chainNode = aItemPtr->GetProxyChain();
      WsfPProxyPath path      = chainNode.GetPath();
      if (!path.Empty())
      {
         std::vector<UtTextDocumentRange> ranges;
         resultsPtr->FindModificationLocationsExtended(path, ranges);
         // if the command chain entry isn't found, try to find the platform
         while (ranges.empty() && !path.Empty())
         {
            path.Pop();
            resultsPtr->FindModificationLocationsExtended(path, ranges);
         }
         wizard::FindReferencesResultDialog::GoToLocationList(mProjectPtr, ranges);
      }
   }
   return opened;
}

void CommandChain::DockWidget::CollapseChildren(const QModelIndex& aIndex)
{
   if (!aIndex.isValid())
   {
      return;
   }

   int childCount = aIndex.model()->rowCount(aIndex);
   for (int i = 0; i < childCount; ++i)
   {
      const QModelIndex& child = aIndex.child(i, 0);
      // Recursively call the function for each child node.
      CollapseChildren(child);
   }

   mUi.commandChainView->collapse(aIndex);
}

void CommandChain::DockWidget::ExpandChildren(const QModelIndex& aIndex)
{
   if (!aIndex.isValid())
   {
      return;
   }

   int childCount = aIndex.model()->rowCount(aIndex);
   for (int i = 0; i < childCount; ++i)
   {
      const QModelIndex& child = aIndex.child(i, 0);
      // Recursively call the function for each child node.
      ExpandChildren(child);
   }

   mUi.commandChainView->expand(aIndex);
}

void CommandChain::DockWidget::ShowPlatformInfo(const WsfPM_Platform& aPlatform, const QPoint& aPos)
{
   wizard::ParseResults* resultsPtr = mProjectPtr->WaitForParseResults();
   if (resultsPtr && aPlatform)
   {
      wizard::EditAssist_Type2* typeAssist = new wizard::EditAssist_Type2();
      // Show just the platform
      typeAssist->SetAssistantData(aPos);
      typeAssist->SetTypeData(*resultsPtr, aPlatform.GetPath());
      typeAssist->show();
   }
}

void CommandChain::DockWidget::AddToNewCommandChain(const std::string& aPlatformName)
{
   wizard::ParseResults* resultsPtr = mProjectPtr->WaitForParseResults();
   if (resultsPtr)
   {
      QString         name = QInputDialog::getText(this, "New Command Chain", "Command Chain Name:");
      WsfPM_Root      proxyRoot{resultsPtr->Proxy()};
      WsfPM_Platform  proxyPlatform{proxyRoot.platforms().FindT(aPlatformName)};
      WsfPM_ObjectMap commandChains{proxyPlatform.Commanders()};
      WsfPProxyNode   commander{commandChains.Find(name.toStdString())};
      if (commander && commander.GetSourceDocumentRange().Valid())
      {
         return;
      }
      else
      {
         wizard::EditorHelpers::AddNewAttributeInText(proxyPlatform,
                                                      "command_chain",
                                                      QString{"%1 %2"}.arg(name).arg(WsfPM_Platform::CommanderSelfValue()));
      }
   }
}

//! Returns the editor that contains the document where the platform definition lies or the last document the platform
//! was edited in (via edit_platform). aRangePtr will be set with the exact location within the document where platform
//! definition (or edit_platform) is.
wizard::Editor* CommandChain::DockWidget::GetEditor(const WsfPProxyNode& aNode, UtTextDocumentRange& aRange)
{
   WsfPProxyIndexNode* proxyIndexNode = aNode.FindIndexNode();
   if (proxyIndexNode && !proxyIndexNode->mEntries.empty())
   {
      if (proxyIndexNode->mEntries.size() ==
          1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
      {
         std::string fileName = aNode.GetSourceDocumentRange().mSource->GetFilePath().GetNormalizedPath();

         aRange.SetRange(aNode.GetSourceDocumentRange().GetBegin(), aNode.GetSourceDocumentRange().GetEnd());
         aRange.mSource = aNode.GetSourceDocumentRange().mSource;
         return wizard::Project::Instance()->GotoFile(fileName);
      }
      else
      {
         for (auto iter = proxyIndexNode->mEntries.rbegin(); iter != proxyIndexNode->mEntries.rend(); ++iter)
         {
            const WsfParseNode* parseNode = iter->mNodePtr;
            if (parseNode && parseNode->GetParent())
            {
               WsfParseNode*       parent   = parseNode->GetParent();
               UtTextDocumentRange curRange = parent->SubtreeRange();
               wizard::Editor*     editorPtr =
                  wizard::Project::Instance()->GotoFile(curRange.mSource->GetFilePath().GetNormalizedPath());

               QTextCursor tmpCursor = editorPtr->textCursor();
               tmpCursor.setPosition(curRange.GetBegin());
               editorPtr->setTextCursor(tmpCursor);
               QString curBlockStr = editorPtr->textCursor().block().text();
               curBlockStr         = curBlockStr.simplified();
               // loop through block().previous() until we get to platform definition or edit platform
               // if edit platform is found, mark this as the document range we want to deal with...
               while (!curBlockStr.startsWith("platform") && !curBlockStr.startsWith("edit platform"))
               {
                  QTextBlock previousBlock   = editorPtr->textCursor().block().previous();
                  curBlockStr                = previousBlock.text().simplified();
                  QTextCursor curBlockCursor = editorPtr->textCursor();
                  curBlockCursor.setPosition(previousBlock.position());
                  editorPtr->setTextCursor(curBlockCursor);
               }
               if (curBlockStr.startsWith("edit platform"))
               {
                  aRange.SetRange(curRange.GetBegin(), curRange.GetEnd());
                  aRange.mSource = curRange.mSource;
                  return editorPtr;
               }
            }
         }
      }
   }
   return nullptr;
}

void CommandChain::DockWidget::showEvent(QShowEvent* e)
{
   assert(mCommandChainModelPtr == nullptr);
   mCommandChainModelPtr = ut::make_unique<Model>();
   mCommandChainModelPtr->setParent(this);
   mUi.commandChainView->setModel(mCommandChainModelPtr.get());
   mCallbacks += mCommandChainModelPtr->CommandChainListChange.Connect(&DockWidget::PopulateCommandChains, this);

   QDockWidget::showEvent(e);
}

void CommandChain::DockWidget::hideEvent(QHideEvent* e)
{
   mUi.commandChainView->setModel(nullptr);
   mCommandChainModelPtr = nullptr;

   QDockWidget::hideEvent(e);
}

void CommandChain::DockWidget::TryCopy()
{
   if (mUi.commandChainView->hasFocus())
   {
      QModelIndexList selection = mUi.commandChainView->selectionModel()->selectedRows(0);
      if (!selection.isEmpty())
      {
         QMimeData* data = mCommandChainModelPtr->mimeData(selection);
         if (data)
         {
            QApplication::clipboard()->setMimeData(data);
         }
      }
   }
}
