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

#include "View.hpp"

#include <cstring>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QTextBlock>
#include <QTextDocumentFragment>

#include "DockWidget.hpp"
#include "DragAndDrop.hpp"
#include "Editor.hpp"
#include "EditorHelpers.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "ParseResults.hpp"
#include "ProjectWorkspace.hpp"
#include "WkfEnvironment.hpp"

CommandChain::View::View(QWidget* aParent)
   : QTreeView(aParent)
   , mParentPtr()
   , mSideRegEx(".*")
   , mCommandChainRegEx(".*")
   , mNeedsFilterUpdate(false)
{
   setDragEnabled(true);
   setDragDropMode(DragDrop);
}

void CommandChain::View::dragEnterEvent(QDragEnterEvent* aEvent)
{
   if (!mParentPtr->Project())
   {
      return QTreeView::dragEnterEvent(aEvent);
   }
   if (aEvent->mimeData()->hasFormat(wizard::DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      aEvent->accept();
   }
   else
   {
      QTreeView::dragEnterEvent(aEvent);
   }
}

void CommandChain::View::dragMoveEvent(QDragMoveEvent* aEvent)
{
   QTreeView::dragMoveEvent(aEvent);

   if (!mParentPtr->Project())
   {
      return;
   }

   bool canCapture = false;
   if (aEvent->mimeData() && aEvent->mimeData()->hasFormat(wizard::DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      QModelIndex idx     = indexAt(aEvent->pos());
      Item*       itemPtr = mParentPtr->FindItem(idx);
      if (itemPtr)
      {
         canCapture = true;
      }
   }
   if (canCapture)
   {
      aEvent->accept();
   }
   else
   {
      aEvent->ignore();
   }
}


void CommandChain::View::dragLeaveEvent(QDragLeaveEvent* aEvent)
{
   QTreeView::dragLeaveEvent(aEvent);
}


void CommandChain::View::dropEvent(QDropEvent* aEvent)
{
   if (!mParentPtr->Project())
   {
      return QTreeView::dropEvent(aEvent);
   }
   if (aEvent->mimeData() && aEvent->mimeData()->hasFormat(wizard::DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      QModelIndex                       idx        = indexAt(aEvent->pos());
      Item*                             futureItem = mParentPtr->FindItem(idx);
      wizard::DragAndDrop::PropertyList propList   = wizard::DragAndDrop::ReadPropertySetMime(aEvent->mimeData());
      std::set<EditWrapper>             sortedEdits;
      for (const auto& currentData : propList)
      {
         WrapCommandChain(sortedEdits, currentData, *futureItem);
      }
      wizEnv.BeginUndoCapture();
      for (auto& elem : sortedEdits)
      {
         elem.Edit();
      }
      wizEnv.EndUndoCapture();
   }
}

void CommandChain::View::contextMenuEvent(QContextMenuEvent* aEvent)
{
   mParentPtr->ContextEvent(currentIndex(), aEvent->globalPos());
}

void CommandChain::View::startDrag(Qt::DropActions aSupportedActions)
{
   QTreeView::startDrag(aSupportedActions);
}

bool CommandChain::View::IsItemFiltered(const QModelIndex& aIndex) const
{
   Model* modelPtr = dynamic_cast<Model*>(model());
   Item*  itemPtr  = dynamic_cast<Item*>(modelPtr->FindItem(aIndex));
   if (!itemPtr)
      return true;
   if (!(itemPtr->GetAbstractItemParent()))
      return false; // never filter the root
   if (itemPtr->IsUnassignedRoot())
      return false; // never filter the 'unassigned' folder
   if (itemPtr->mIsPlatform)
   {
      if (!mSideRegEx.exactMatch(itemPtr->Side().c_str()))
      {
         return true;
      }
   }
   if (!mCommandChainRegEx.exactMatch(itemPtr->mCommandChain.c_str()))
   {
      return true;
   }
   return false;
}

void CommandChain::View::rowsInserted(const QModelIndex& aParent, int aStart, int aEnd)
{
   QTreeView::rowsInserted(aParent, aStart, aEnd);
   Model* modelPtr = dynamic_cast<Model*>(model());
   if (modelPtr)
   {
      if (modelPtr->data(aParent, Qt::UserRole).toBool())
      {
         expand(aParent);
      }

      for (int row = aStart; row <= aEnd; ++row)
      {
         bool hideItem = IsItemFiltered(modelPtr->index(row, 0, aParent));
         if (hideItem)
         {
            setRowHidden(row, aParent, hideItem);
         }
      }
   }
}

void CommandChain::View::dataChanged(const QModelIndex& aTopLeft, const QModelIndex& aBottomRight)
{
   QTreeView::dataChanged(aTopLeft, aBottomRight);
   Model* modelPtr = static_cast<Model*>(model());
   for (int row = aTopLeft.row(); row <= aBottomRight.row(); ++row)
   {
      QModelIndex parent   = aTopLeft.parent();
      bool        hideItem = IsItemFiltered(modelPtr->index(row, 0, parent));
      setRowHidden(row, parent, hideItem);
   }
}

void CommandChain::View::FilterUpdate()
{
   if (mNeedsFilterUpdate)
   {
      mNeedsFilterUpdate = false;
      Model* modelPtr    = static_cast<Model*>(model());
      FilterUpdateP(modelPtr->GetRootItem());
   }
}
void CommandChain::View::FilterUpdateP(Item* aItemPtr)
{
   Model*      modelPtr = static_cast<Model*>(model());
   QModelIndex idx      = modelPtr->GetRowIndex(*aItemPtr);
   if (IsItemFiltered(idx))
   {
      setRowHidden(idx.row(), idx.parent(), true);
   }
   else
   {
      setRowHidden(idx.row(), idx.parent(), false);
      for (int i = 0; i < aItemPtr->GetAbstractItemCount(); ++i)
      {
         FilterUpdateP(static_cast<Item*>(aItemPtr->GetAbstractItem(i)));
      }
   }
}

void CommandChain::View::FilterUpdateLater()
{
   if (!mNeedsFilterUpdate)
   {
      mNeedsFilterUpdate = true;
      QTimer::singleShot(100, this, &View::FilterUpdate);
   }
}

void CommandChain::View::WrapCommandChain(std::set<EditWrapper>& aSortedEdits,
                                          const QVariantMap&     aData,
                                          const Item&            aItem) const noexcept
{
   // Get the old command chain name
   std::string oldCommandChainName;
   if (aData.contains("command-chain"))
   {
      oldCommandChainName = aData.value("command-chain").toString().toStdString();
   }
   // Get the new command chain name
   std::string newCommandChainName{aItem.mCommandChain};
   // Get the input command
   // The input command is "command_chain" if the command chain is not default
   // Otherwise, the command is "commander"
   QString inputCommand{"command_chain"};
   if (newCommandChainName == "default")
   {
      inputCommand = "commander";
   }
   // Get the new commander name
   std::string newCommanderName{aItem.mPlatform.GetName()};
   // If the drop item is the cUNASSIGNED_ROOT, then set the new comamnder name to that
   if (aItem.mName == Item::cUNASSIGNED_ROOT)
   {
      newCommanderName = Item::cUNASSIGNED_ROOT;
   }
   // If the drop item is a command chain, then set the new commander name to SELF
   else if (newCommanderName.empty())
   {
      newCommanderName = WsfPM_Platform::CommanderSelfValue();
   }
   if (aData.contains("proxy-node"))
   {
      bool           wrapped{false};
      bool           needToDelete{newCommanderName == Item::cUNASSIGNED_ROOT};
      WsfPM_Platform proxyPlatform{wizard::DragAndDrop::ToProxyNode(aData.value("proxy-node").toString())};
      if (proxyPlatform)
      {
         std::string platformName{proxyPlatform.GetName()};
         // Ignore requests when the drop item is the same as the drag item
         if (aItem.mIsPlatform && !aItem.mIsUnassignedRoot && aItem.mName == platformName)
         {
            return;
         }
         // If the command chain name and commander name are the same, then set the new commander name to SELF
         else if (platformName == newCommandChainName && platformName == newCommanderName)
         {
            newCommanderName = WsfPM_Platform::CommanderSelfValue();
         }
         WsfPM_ObjectMap commandChains{proxyPlatform.Commanders()};
         if (commandChains)
         {
            WsfPProxyNode commander{commandChains.Find(newCommandChainName)};
            if (commander)
            {
               std::string oldCommanderName{commander.GetValue().ValueToString()};
               // Wrap the commander node to delete later
               // As to allow any "Unassigned Platforms" item to delete a command chain, this is to handle the
               // "Unassigned Platforms" item under the SAME command chain item as the dragged item.
               if (needToDelete)
               {
                  aSortedEdits.emplace(EditWrapper::Operation::Delete, commander, "", "", "");
               }
               // Wrap the commander node to edit later with the new command chain and commander
               else if (oldCommanderName != newCommanderName)
               {
                  aSortedEdits.emplace(EditWrapper::Operation::Edit,
                                       commander,
                                       inputCommand,
                                       QString::fromStdString(newCommandChainName),
                                       QString::fromStdString(newCommanderName));
               }
               wrapped = true;
            }
            commander = commandChains.Find(oldCommandChainName);
            if (!wrapped && commander)
            {
               // Wrap the commander node to delete later
               // As to allow any "Unassigned Platforms" item to delete a command chain, this is to handle the
               // "Unassigned Platforms" item under a DIFFERENT command chain item as the dragged item.
               if (needToDelete)
               {
                  aSortedEdits.emplace(EditWrapper::Operation::Delete, commander, "", "", "");
               }
               wrapped = true;
            }
         }
         // Wrap the commander node to add later
         if (!wrapped && !needToDelete)
         {
            aSortedEdits.emplace(EditWrapper::Operation::Add,
                                 proxyPlatform,
                                 inputCommand,
                                 QString::fromStdString(newCommandChainName),
                                 QString::fromStdString(newCommanderName));
         }
      }
   }
}

void CommandChain::View::EditWrapper::Edit() const noexcept
{
   // Perform the edit operation
   switch (mOperation)
   {
   case Operation::Add:
      AddCommandChain();
      break;
   case Operation::Delete:
      DeleteCommandChain();
      break;
   case Operation::Edit:
      EditCommandChain();
      break;
   default:
      break;
   }
}

void CommandChain::View::EditWrapper::AddCommandChain() const noexcept
{
   // Add the "commander <platform_name>" into the platform definition
   if (mCommandChainName == "default")
   {
      wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mInputCommand, mCommanderName);
   }
   // Add the "command_chain <command_chain_name> <platform_name>" command into the platform definition
   else
   {
      wizard::EditorHelpers::AddNewAttributeInText(GetProxyNode(), mInputCommand, mCommandChainName + ' ' + mCommanderName);
   }
}

void CommandChain::View::EditWrapper::DeleteCommandChain() const noexcept
{
   // Go to the location of the command chain
   QTextCursor commandChainCursor{GetEditor()->textCursor()};
   commandChainCursor.setPosition(GetRange().GetBegin());
   // Consume the whitespace before the range
   wizard::EditorHelpers::MoveToPreviousNonWhitespaceToken(commandChainCursor, QTextCursor::KeepAnchor);
   commandChainCursor.movePosition(QTextCursor::EndOfWord);
   commandChainCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
   // Remove the command chain from the platform definition
   wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), commandChainCursor, "");
}

void CommandChain::View::EditWrapper::EditCommandChain() const noexcept
{
   // Go to the location of the command chain
   QTextCursor commandChainCursor{GetEditor()->textCursor()};
   commandChainCursor.setPosition(GetRange().GetBegin());
   commandChainCursor.setPosition(GetRange().GetEnd() + 1, QTextCursor::KeepAnchor);
   // Extract the command chain text
   QString     commandChainText{commandChainCursor.selection().toPlainText()};
   QStringList commandChainSplit{wizard::Editor::Split(commandChainText)};
   bool        edited{false};
   if (!commandChainSplit.isEmpty())
   {
      // Read the input command, which is "commander" or "command_chain"
      int commandIndex{commandChainSplit.indexOf(mInputCommand)};
      if (commandIndex >= 0)
      {
         // Read the command chain name
         int commandChainIndex{wizard::Editor::FindNextValidToken(commandChainSplit, commandIndex + 1)};
         // This is an instance of the "command_chain" command
         if (commandChainIndex >= 0)
         {
            // Edit the command chain name
            commandChainSplit[commandChainIndex] = mCommandChainName;
            // Read the commander name
            // Note:  This will fail for the "commander" command and will fall through to the next commander routine to
            // update the commander name.
            int commanderIndex{wizard::Editor::FindNextValidToken(commandChainSplit, commandChainIndex + 1)};
            if (commanderIndex >= 0)
            {
               // Edit the commander name
               commandChainSplit[commanderIndex] = mCommanderName;
               edited                            = true;
            }
         }
         // This is an instance of the "commander" command
         // Read the commander name
         int commanderIndex{wizard::Editor::FindNextValidToken(commandChainSplit, commandIndex + 1)};
         if (!edited && commanderIndex >= 0)
         {
            // Edit the commander name
            commandChainSplit[commanderIndex] = mCommanderName;
            edited                            = true;
         }
      }
   }
   // Replace the old heading text with the new heading text
   if (edited)
   {
      wizard::EditorHelpers::InsertTextAndResetCursor(GetEditor(), commandChainCursor, commandChainSplit.join(""));
   }
}
