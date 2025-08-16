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
#ifndef PASTECONTEXT_HPP
#define PASTECONTEXT_HPP

#include <string>

#include <QString>

#include "UtLatPos.hpp"
#include "UtLonPos.hpp"
#include "ViExport.hpp"
#include "WsfPProxyNode.hpp"

class QWidget;
class QMimeData;

namespace wizard
{
//! The same object types can be pasted in multiple contexts,
//! this class represents the context of a paste operation
class VI_EXPORT PasteContext
{
public:
   explicit PasteContext(const std::string& aContextName);

   bool Exec() const;

   bool Exec(const QMimeData* data) const;

   void SetDropOperation() { mIsDropOperation = true; }
   bool IsDropOperation() const { return mIsDropOperation; }
   void SetWidget(QWidget* aWidgetPtr) { mWidgetPtr = aWidgetPtr; }
   // The widget being pasted to
   QWidget* GetWidget() const { return mWidgetPtr; }
   // A name for the context.  eg: "map", "text-editor", etc...
   std::string     GetContextName() const { return mContextName; }
   void            SetPosition(const UtLatPos& aLat, const UtLonPos& aLon);
   bool            HasPosition() const { return mHasPosition; }
   const UtLatPos& GetLat() const { return mLat; }
   const UtLonPos& GetLon() const { return mLon; }
   void            SetFile(const std::string& aFilePath);
   std::string     GetFilePath() const;
   // The proxy node being pasted-to
   WsfPProxyNode GetProxyNode() const;
   void          SetProxyNode(const WsfPProxyNode& aNode);

   // Add an available paste option.
   // If aIsGrouped is 'true' for all options,
   // the paste will be seen as a single action to the user.
   // If aIsGrouped is 'false', then this action will appear as an independent action
   // available from a subsequent popup-menu.
   // Label should refer to the object being pasted, paste labels are combined when aIsGrouped=true
   //   resulting in 'Paste platform(5), zone(2)...'
   void AddAvailableAction(const std::string& aName, QString aLabel, bool aIsGrouped = true) const;
   // Returns 'true' if the named action was chosen.
   bool IsActionChosen(const std::string& aName) const;

   bool ApplyToTextImmediately() const { return mApplyToTextImmediately; }
   void ApplyToTextImmediately(bool aApplyNow) { mApplyToTextImmediately = aApplyNow; }

private:
   struct PasteAction
   {
      std::string mName;
      QString     mLabel;
      bool        mIsGrouped;
      bool        mIsChosen;
   };

   mutable std::vector<PasteAction> mActionList;
   QWidget*                         mWidgetPtr;
   std::string                      mContextName;
   WsfPProxyNode                    mNode;
   bool                             mHasPosition;
   bool                             mIsDropOperation;
   bool                             mApplyToTextImmediately;
   UtLatPos                         mLat;
   UtLonPos                         mLon;
   std::string                      mFilePath;
};
} // namespace wizard
#endif
