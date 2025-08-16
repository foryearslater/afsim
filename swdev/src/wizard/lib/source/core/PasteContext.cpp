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

#include "PasteContext.hpp"

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QMenu>
#include <QMimeData>

#include "Signals.hpp"
#include "Undo.hpp"


void wizard::PasteContext::SetProxyNode(const WsfPProxyNode& aNode)
{
   mNode = aNode;
}

WsfPProxyNode wizard::PasteContext::GetProxyNode() const
{
   return mNode;
}

//! Returns empty string if no file path given
std::string wizard::PasteContext::GetFilePath() const
{
   return mFilePath;
}

void wizard::PasteContext::SetFile(const std::string& aFilePath)
{
   mFilePath = aFilePath;
}

void wizard::PasteContext::SetPosition(const UtLatPos& aLat, const UtLonPos& aLon)
{
   mLat = aLat;
   mLon = aLon;
}

wizard::PasteContext::PasteContext(const std::string& aContextName)
   : mWidgetPtr(nullptr)
   , mHasPosition(false)
   , mIsDropOperation(false)
   , mApplyToTextImmediately(false)
{
}

bool wizard::PasteContext::Exec() const
{
   const QMimeData* data = QApplication::clipboard()->mimeData();
   return Exec(data);
}

bool wizard::PasteContext::Exec(const QMimeData* aData) const
{
   wizSignals->PasteQuery(aData, *this, false);
   if (mActionList.empty())
   {
      return false;
   }

   bool    needPopup     = false;
   QString defaultOption = "Paste ";
   for (auto&& action : mActionList)
   {
      if (!action.mIsGrouped)
         needPopup = true;
      else
      {
         defaultOption += action.mLabel;
      }
   }

   if (needPopup)
   {
      QMenu    pop;
      QAction* defaultAction = pop.addAction(defaultOption);
      for (auto&& action : mActionList)
      {
         if (!action.mIsGrouped)
         {
            QAction* act = pop.addAction("Paste " + action.mLabel);
            act->setData(QString::fromStdString(action.mName));
         }
      }
      QAction* actionPtr = pop.exec(QCursor::pos());
      if (actionPtr)
      {
         // Mark actions as chosen
         if (actionPtr == defaultAction)
         {
            for (auto&& action : mActionList)
            {
               if (action.mIsGrouped)
               {
                  action.mIsChosen = true;
               }
            }
         }
         else
         {
            for (auto&& action : mActionList)
            {
               if (action.mName == actionPtr->data().toString().toStdString())
               {
                  action.mIsChosen = true;
               }
            }
         }
         UndoContext undoCtx(actionPtr->text().toStdString().c_str());
         // Dispatch
         wizSignals->PasteQuery(aData, *this, true);
         return true;
      }
      else
      {
         // no action clicked...
         return false;
      }
   }
   else
   {
      // all actions are grouped, don't use popup and choose all
      for (auto&& action : mActionList)
      {
         action.mIsChosen = true;
      }
      UndoContext undoCtx(defaultOption.toStdString().c_str());
      // Dispatch
      wizSignals->PasteQuery(aData, *this, true);
      return true;
   }
   // never get here
}

void wizard::PasteContext::AddAvailableAction(const std::string& aName, QString aLabel, bool aIsGrouped /*= true*/) const
{
   PasteAction a = {aName, aLabel, aIsGrouped, false};
   mActionList.push_back(a);
}

bool wizard::PasteContext::IsActionChosen(const std::string& aName) const
{
   for (auto&& action : mActionList)
   {
      if (action.mName == aName)
         return action.mIsChosen;
   }
   return false;
}
