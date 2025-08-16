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

#ifndef VIEW_HPP
#define VIEW_HPP

#include <set>

#include <QTreeView>

#include "ChangeWrapper.hpp"
#include "UtTextDocument.hpp"

class WsfPM_Platform;
class WsfPM_Root;

namespace wizard
{
class Editor;
}

namespace CommandChain
{
class DockWidget;
class Item;

class View final : public QTreeView
{
   Q_OBJECT

public:
   explicit View(QWidget* aParent);

   void dragEnterEvent(QDragEnterEvent* aEvent) override;
   void dragMoveEvent(QDragMoveEvent* aEvent) override;

   void         dragLeaveEvent(QDragLeaveEvent* aEvent) override;
   void         dropEvent(QDropEvent* aEvent) override;
   void         rowsInserted(const QModelIndex& aParent, int aStart, int aEnd) override;
   virtual void dataChanged(const QModelIndex& aTopLeft, const QModelIndex& aBottomRight);

   void FilterUpdateLater();

   DockWidget* mParentPtr;
   QRegExp     mSideRegEx;
   QRegExp     mCommandChainRegEx;
   bool        mNeedsFilterUpdate;

protected slots:

   void FilterUpdate();

protected:
   void FilterUpdateP(Item* aItemPtr);
   void contextMenuEvent(QContextMenuEvent* aEvent) override;

   bool IsItemFiltered(const QModelIndex& aIndex) const;
   void startDrag(Qt::DropActions aSupportedActions) override;

   //! Specialization of wizard::ChangeWrapper to handle command-chain-related changes to the text
   class EditWrapper final : public wizard::ChangeWrapper
   {
   public:
      //! Represents the operation to perform to the text
      enum class Operation
      {
         Add,    //!< add the command chain
         Delete, //!< delete the command chain
         Edit    //!< edit the command chain
      };

      //! Construct an EditWrapper
      //! @param aOperation is the edit operation
      //! @param aProxyNode is the proxy property to modify
      //! @param aInputCommand is the appropriate command chain input command to use
      //! @param aCommandChainName is the new command chain name
      //! @param aCommanderName is the new commander name
      EditWrapper(const Operation aOperation,
                  WsfPProxyNode   aProxyNode,
                  const QString&  aInputCommand,
                  const QString&  aCommandChainName,
                  const QString&  aCommanderName)
         : ChangeWrapper(std::move(aProxyNode))
         , mOperation(aOperation)
         , mInputCommand(aInputCommand)
         , mCommandChainName(aCommandChainName)
         , mCommanderName(aCommanderName)
      {
      }

      //! Perform the edit operation
      void Edit() const noexcept;

   protected:
      //! Add the command chain into the platform definition
      void AddCommandChain() const noexcept;
      //! Edit the command chain in the platform definition
      void DeleteCommandChain() const noexcept;
      //! Delete the command chain from the platform definition
      void EditCommandChain() const noexcept;

      //! The edit operation
      const Operation mOperation;
      //! The input command, which is either "commander" or "command_chain"
      QString mInputCommand;
      //! The command chain name, which is either default or any other string
      QString mCommandChainName;
      //! The commander name, which is a platform name
      QString mCommanderName;
   };

   //! Called by View::dropEvent to wrap each command chain change.
   //! @param aData is the QVariantMap referencing the current command chain information
   //! @param aItem is the item referencing the new command chain information
   void WrapCommandChain(std::set<EditWrapper>& aSortedEdits, const QVariantMap& aData, const Item& aItem) const noexcept;
};
} // namespace CommandChain

#endif
