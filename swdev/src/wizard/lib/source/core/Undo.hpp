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
#ifndef UNDO_HPP
#define UNDO_HPP

#include <stack>
#include <string>

#include <QUndoCommand>

#include "ViExport.hpp"
#include "WsfPM_Platform.hpp"
#include "WsfPProxy.hpp"
class QAction;
class WsfPProxyPath;

namespace wizard
{
class UndoContext;
class UndoProxyChange;

class VI_EXPORT Undo
{
public:
   enum Flags
   {
      cDISCARD_PROXY_CHANGES = 1
   };
   friend class UndoContext;

   static Undo* Instance() { return sInstance; }
   static void  Destroy()
   {
      delete sInstance;
      sInstance = nullptr;
   }

   Undo();
   Undo(const Undo&) = delete;            // Copy
   Undo(Undo&&)      = default;           // Move
   Undo& operator=(const Undo&) = delete; // Copy assignment
   Undo& operator=(Undo&&) = default;     // Move assignment
   ~Undo();

   QAction* CreateUndoAction(QObject* aParent);

   QAction* CreateRedoAction(QObject* aParent);

   void BeforeProxyChange(WsfPProxy*                     aProxyPtr,
                          WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                          const WsfPProxyPath&           aPath,
                          const std::string*             aRenameName);

   QUndoCommand* CurrentCommand() const;

   void        PushUndo(QUndoCommand* aUndoPtr);
   QUndoStack* UndoStack() const { return mUndoStack; }

   void        ClearProxyChanges();
   void        BeginDiscardProxyChanges() { ++mDiscardProxyChanges; }
   void        EndDiscardProxyChanges() { --mDiscardProxyChanges; }
   QUndoStack* GetUndoStack() { return mUndoStack; }

private:
   static Undo*  sInstance;
   QUndoStack*   mUndoStack;
   QUndoCommand* mCurrentGroup;
   int           mUndoContextCount;
   int           mUndoFlags;
   int           mDiscardProxyChanges;
};

class VI_EXPORT UndoProxyChange : public QUndoCommand
{
public:
   enum Action
   {
      cADD_CHILD,
      cREMOVE_CHILD,
      cASSIGN_VALUE,
      cRENAME_CHILD,
      cNULL_ACTION
   };

   static UndoProxyChange* createUndo(WsfPProxy*                     aProxyPtr,
                                      WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                                      const WsfPProxyPath&           aPath,
                                      const std::string*             aRenameName);

   ~UndoProxyChange() override;

   void undo() override;
   void redo() override;

private:
   UndoProxyChange(QUndoCommand* aParent, Action aAction, const WsfPProxyPath& aPath);

protected:
   // Toggles the action.  Can implement undo and redo
   void           UndoRedo(WsfPProxy* aProxyPtr);
   bool           mIsUndone;
   WsfPProxy*     mProxyPtr;
   Action         mAction;
   WsfPProxyPath  mChangePath;
   WsfPProxyValue mSwapValue;
   // for cRENAME_CHILD, mSwapName is the final name
   std::string mSwapName;
};

// Groups undo events between constructor and destructor into a single action
class VI_EXPORT UndoContext
{
public:
   explicit UndoContext(const char* aUndoDescription = nullptr);
   explicit UndoContext(const std::string& aUndoDescription);
   explicit UndoContext(const QString& aUndoDescription);
   ~UndoContext();
   QUndoCommand* ParentCommand() const;
   void          SetFlags(int aFlags);
   void          ClearFlags(int aFlags);
   void          End();
   void          Push(QUndoCommand* aUndoPtr) { Undo::Instance()->PushUndo(aUndoPtr); }

private:
   void Init(const QString& aUndoDescription);

   bool mEnded;
   int  mFlagsSet;
   int  mFlagsCleared;
   // the flags that need to be toggled when this context goes out of scope
   int mFlagsToggled;
};
} // namespace wizard
#endif
