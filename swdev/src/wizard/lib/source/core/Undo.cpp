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
#include "Undo.hpp"

#include <QAction>
#include <QUndoStack>

#include "WsfPM_ObjectMap.hpp"

wizard::Undo* wizard::Undo::sInstance = nullptr;
wizard::Undo::Undo()
   : mUndoStack(new QUndoStack)
{
   mDiscardProxyChanges = 0;
   mCurrentGroup        = nullptr;
   mUndoContextCount    = 0;
   assert(sInstance == nullptr);
   sInstance  = this;
   mUndoFlags = 0;
}

wizard::Undo::~Undo()
{
   delete mUndoStack;
}

void wizard::Undo::BeforeProxyChange(WsfPProxy*                     aProxyPtr,
                                     WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                                     const WsfPProxyPath&           aPath,
                                     const std::string*             aRenameName)
{
   if ((mUndoFlags & cDISCARD_PROXY_CHANGES) == 0)
   {
      if (mDiscardProxyChanges == 0)
      {
         UndoProxyChange* undoPtr = UndoProxyChange::createUndo(aProxyPtr, aProxyModifiedReason, aPath, aRenameName);
         if (undoPtr)
         {
            PushUndo(undoPtr);
         }
      }
   }
}

QUndoCommand* wizard::Undo::CurrentCommand() const
{
   return mCurrentGroup;
}

void wizard::Undo::PushUndo(QUndoCommand* aUndoPtr)
{
   if (mCurrentGroup == aUndoPtr)
   {
      mCurrentGroup = nullptr;
      PushUndo(aUndoPtr);
   }
   else if (mCurrentGroup == nullptr)
   {
      mUndoStack->push(aUndoPtr);
   }
}

void wizard::Undo::ClearProxyChanges()
{
   mUndoStack->clear();
}

QAction* wizard::Undo::CreateUndoAction(QObject* aParent)
{
   QAction* undoActionPtr = mUndoStack->createUndoAction(aParent);
   /*   undoActionPtr->setShortcut(ViActionSet::GetInstance().GetAction("Edit.Do.Undo")->shortcut());
      undoActionPtr->setIcon(ViActionSet::GetInstance().GetAction("Edit.Do.Undo")->icon());*/
   return undoActionPtr;
}

QAction* wizard::Undo::CreateRedoAction(QObject* aParent)
{
   QAction* redoActionPtr = mUndoStack->createRedoAction(aParent);
   /*   redoActionPtr->setShortcut(ViActionSet::GetInstance().GetAction("Edit.Do.Redo")->shortcut());
      redoActionPtr->setIcon(ViActionSet::GetInstance().GetAction("Edit.Do.Redo")->icon());*/
   return redoActionPtr;
}


/*
   Proxy undo:
   Here we are intercepting notifications *before* proxy changes happen, and creating undo actions
   Many of these do a 'swap' which avoids copying the value being overwritten
*/

wizard::UndoProxyChange* wizard::UndoProxyChange::createUndo(WsfPProxy*                     aProxyPtr,
                                                             WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                                                             const WsfPProxyPath&           aPath,
                                                             const std::string*             aRenameName)
{
   switch (aProxyModifiedReason)
   {
   case WsfPProxy::cPM_CHILD_ADDED:
   {
      UndoProxyChange* change = new UndoProxyChange(Undo::Instance()->CurrentCommand(), cREMOVE_CHILD, aPath);
      change->mProxyPtr       = aProxyPtr;
      return change;
   }
   case WsfPProxy::cPM_CHILD_REMOVED:
   {
      WsfPProxyValue container = aProxyPtr->mRoot.Lookup(aPath, (int)aPath.size() - 1);
      if (container)
      {
         WsfPProxyValue swapVal;
         if (container.SwapAttr(aPath.Back(), swapVal))
         {
            UndoProxyChange* change = new UndoProxyChange(Undo::Instance()->CurrentCommand(), cADD_CHILD, aPath);
            change->mSwapValue      = swapVal;
            change->mProxyPtr       = aProxyPtr;
            return change;
         }
      }
      return nullptr;
   }
   case WsfPProxy::cPM_VALUE_CHANGED:
   { // a value is going to change.  capture its value now for restoration later
      WsfPProxyValue container = aProxyPtr->mRoot.Lookup(aPath, (int)aPath.size() - 1);
      if (container)
      {
         WsfPProxyValue swapVal;
         if (container.SwapAttr(aPath.Back(), swapVal))
         {
            UndoProxyChange* change = new UndoProxyChange(Undo::Instance()->CurrentCommand(), cASSIGN_VALUE, aPath);
            change->mSwapValue      = swapVal;
            change->mProxyPtr       = aProxyPtr;
            return change;
         }
      }
      return nullptr;
   }
   case WsfPProxy::cPM_CHILD_MOVED:
   {
      WsfPProxyValue      container = aProxyPtr->mRoot.Lookup(aPath, (int)aPath.size() - 1);
      WsfPProxyObjectMap* mapPtr    = container.GetObjectMap();
      if (mapPtr)
      {
         std::string oldName = aPath.Back().GetMapKey();
         if (!mapPtr->FindAt(*aRenameName) && mapPtr->FindAt(oldName))
         {
            WsfPProxyPath path(aPath);
            path.Back().SetMapKey(*aRenameName);
            UndoProxyChange* change = new UndoProxyChange(Undo::Instance()->CurrentCommand(), cRENAME_CHILD, path);
            change->mSwapName       = oldName;
            change->mProxyPtr       = aProxyPtr;
            return change;
         }
      }
      return nullptr;
   }
   default:
      // WIZARD_TODO: add list operations...
      return nullptr;
   }
}

void wizard::UndoProxyChange::UndoRedo(WsfPProxy* aProxyPtr)
{
   switch (mAction)
   {
   case cADD_CHILD:
   {
      WsfPProxyNode container;
      container.MoveTo(aProxyPtr, mChangePath, (int)mChangePath.size() - 1);
      WsfPProxyObjectMap* mapPtr = container.GetValue().GetObjectMap();
      if (mapPtr)
      {
         if (mapPtr->AddNew(mChangePath.Back().GetMapKey(), mSwapValue))
         {
            container += mChangePath.Back();
            container.NotifyValueAdded();
            mSwapValue = WsfPProxyValue();
            mAction    = cREMOVE_CHILD;
         }
         else
         {
            mAction = cNULL_ACTION;
         }
      }
      else
      {
         WsfPProxyList* listPtr = container.GetValue().GetList();
         if (listPtr)
         {
            size_t idx = mChangePath.Back().GetIndex();
            if (idx <= listPtr->Size())
            {
               listPtr->Insert(mSwapValue, idx);
               mSwapValue = WsfPProxyValue();
               container += mChangePath.Back();
               container.NotifyValueAdded();
               mAction = cREMOVE_CHILD;
            }
            else
            {
               mAction = cNULL_ACTION;
            }
         }
      }
      break;
   }
   case cREMOVE_CHILD:
   {
      WsfPProxyNode container;
      container.MoveTo(aProxyPtr, mChangePath, (int)mChangePath.size() - 1);
      WsfPProxyValue      containerValue = container.GetValue();
      WsfPProxyObjectMap* mapPtr         = containerValue.GetObjectMap();
      if (mapPtr)
      {
         const std::string& key = mChangePath.Back().GetMapKey();
         if (containerValue.SwapAttr(key, mSwapValue))
         {
            mapPtr->Erase(key);
            (container + key).NotifyValueRemoved();
            mAction = cADD_CHILD;
         }
         else
         {
            mAction = cNULL_ACTION;
         }
      }
      else
      {
         WsfPProxyList* listPtr = containerValue.GetList();
         if (listPtr)
         {
            size_t idx = mChangePath.Back().GetIndex();
            if (idx < listPtr->Size())
            {
               mSwapValue = listPtr->RemoveNoErase(idx);
               (container + idx).NotifyValueRemoved();
               mAction = cADD_CHILD;
            }
            else
            {
               mAction = cNULL_ACTION;
            }
         }
      }
      break;
   }
   case cASSIGN_VALUE:
   {
      WsfPProxyNode container;
      container.MoveTo(aProxyPtr, mChangePath, (int)mChangePath.size() - 1);
      WsfPProxyValue containerVal = container.GetValue();
      if (containerVal)
      {
         containerVal.SwapAttr(mChangePath.Back(), mSwapValue);
         container += mChangePath.Back();
         container.NotifyValueChanged();
         // redo action is the same
      }
      break;
   }
   case cRENAME_CHILD:
   {
      WsfPProxyNode container;
      container.MoveTo(aProxyPtr, mChangePath, (int)mChangePath.size() - 1);
      WsfPProxyValue      containerVal = container.GetValue();
      WsfPProxyObjectMap* mapPtr       = containerVal.GetObjectMap();
      if (mapPtr)
      {
         if (mapPtr->FindAt(mSwapName) || !mapPtr->FindAt(mChangePath.Back().GetMapKey()))
         {
            mAction = cNULL_ACTION;
         }
         else
         {
            WsfPProxyPath oldPath = mChangePath;
            WsfPProxyPath newPath = WsfPProxyPath(mChangePath, (int)mChangePath.size() - 1);
            newPath += mSwapName;
            mProxyPtr->RecordObjectRename(oldPath, newPath);
            WsfPProxyNode newNode(mProxyPtr, newPath);

            // std::swap(mSwapName, mChangePath.Back().mMapKey());
            std::string name = mSwapName;
            mSwapName        = mChangePath.Back().GetMapKey();
            mChangePath.Back().SetMapKey(name);

            newNode.NotifyValueMoved(mSwapName);
         }
      }
      else
      {
         mAction = cNULL_ACTION;
      }
   }
   case cNULL_ACTION:
      break;
   }
}

wizard::UndoProxyChange::UndoProxyChange(QUndoCommand* aParent, Action aAction, const WsfPProxyPath& aPath)
   : QUndoCommand(aParent)
   , mIsUndone(false)
   , mAction(aAction)
   , mChangePath(aPath)
{
}

wizard::UndoProxyChange::~UndoProxyChange()
{
   mSwapValue.Delete();
}

void wizard::UndoProxyChange::undo()
{
   UndoRedo(mProxyPtr);
   mIsUndone = true;
}

void wizard::UndoProxyChange::redo()
{
   if (mIsUndone)
      UndoRedo(mProxyPtr);
   mIsUndone = false;
}

wizard::UndoContext::UndoContext(const char* aUndoDescription)
{
   Init(QString(aUndoDescription));
}

wizard::UndoContext::UndoContext(const std::string& aUndoDescription)
{
   Init(QString::fromStdString(aUndoDescription));
}

wizard::UndoContext::UndoContext(const QString& aUndoDescription)
{
   Init(aUndoDescription);
}

wizard::UndoContext::~UndoContext()
{
   End();
}

void wizard::UndoContext::End()
{
   if (mEnded)
      return;
   mEnded         = true;
   Undo* gUndoPtr = Undo::Instance();
   gUndoPtr->mUndoFlags ^= mFlagsToggled;
   if (--gUndoPtr->mUndoContextCount == 0)
   {
      assert(gUndoPtr->mCurrentGroup);
      if (gUndoPtr->mCurrentGroup->childCount() == 0)
      {
         delete gUndoPtr->mCurrentGroup;
      }
      else
      {
         Undo::Instance()->PushUndo(gUndoPtr->mCurrentGroup);
      }
      gUndoPtr->mCurrentGroup = nullptr;
      gUndoPtr->mUndoFlags    = 0; // should be 0 already
   }
   else
   {
      assert(gUndoPtr->mCurrentGroup);
   }

   mFlagsToggled = 0;
   mFlagsSet     = 0;
   mFlagsCleared = 0;
}

void wizard::UndoContext::Init(const QString& aUndoDescription)
{
   mEnded        = false;
   mFlagsSet     = 0;
   mFlagsCleared = 0;
   mFlagsToggled = 0;

   Undo* gUndoPtr = Undo::Instance();
   if (++gUndoPtr->mUndoContextCount == 1)
   {
      gUndoPtr->mCurrentGroup = new QUndoCommand(aUndoDescription);
   }
}

void wizard::UndoContext::SetFlags(int aFlags)
{
   int& currentFlags = Undo::Instance()->mUndoFlags;
   mFlagsToggled ^= (mFlagsSet ^ currentFlags) & mFlagsSet;
   mFlagsSet = aFlags;
}
void wizard::UndoContext::ClearFlags(int aFlags)
{
   // int& currentFlags = Undo::instance()->mUndoFlags;
   mFlagsToggled ^= (mFlagsCleared ^ aFlags) & mFlagsCleared;
   mFlagsCleared = aFlags;
}

QUndoCommand* wizard::UndoContext::ParentCommand() const
{
   return Undo::Instance()->mCurrentGroup;
}
