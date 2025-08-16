// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SCRIPTDEBUGGER_HPP
#define SCRIPTDEBUGGER_HPP

#include <vector>

#include "TextSource.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

namespace wizard
{

class ScriptDebugger
{
public:
   struct CallstackEntry
   {
      int64_t     mScriptHandle;
      TextSource* mSourcePtr;
      size_t      mLineNumber;
   };

   class Breakpoint
   {
   public:
      Breakpoint();

      bool operator==(const Breakpoint& aRhs) const
      {
         return mId == aRhs.mId && mPosition == aRhs.mPosition && mPath == aRhs.mPath &&
                mCondition == aRhs.mCondition && mIsEnabled == aRhs.mIsEnabled;
      }
      int         mId;                   // unique ID for the breakpoint
      size_t      mPosition  = ut::npos; // character position of the breakpoint
      TextSource* mSourcePtr = nullptr;  // pointer to the Text Source
      std::string mPath;                 // path of the file name the breakpoint is in
      std::string mCondition;
      size_t      mActualLine = ut::npos; // line the breakpoint is on after the debugger assigns it to an instruction
      // -1 if the breakpoint is not yet active
      bool mToggleLocked   = false; // controls if breakpoint is able to be toggled
      bool mWasUserEnabled = false; // saved state of breakpoint before debug modifications

      bool IsEnabled() const noexcept { return mIsEnabled; }

      void SetIsEnabled(bool aIsEnabled)
      {
         if (!mToggleLocked)
         {
            mIsEnabled = aIsEnabled;
         }
      }

      size_t GetLine() const
      {
         if (mSourcePtr != nullptr)
         {
            return mSourcePtr->PositionToLineNumber(mPosition);
         }
         return 0;
      }

      void SetPositionByLineNumber(size_t aLine)
      {
         if (mSourcePtr != nullptr)
         {
            mPosition = mSourcePtr->GetLinePosition(aLine);
         }
         else
         {
            mPosition = 0;
         }
      }

   private:
      bool mIsEnabled = true;
   };

   virtual ~ScriptDebugger() = default;

   virtual void                         StepInto()                    = 0;
   virtual void                         StepOut()                     = 0;
   virtual void                         StepOver()                    = 0;
   virtual void                         Resume()                      = 0;
   virtual WsfXIO_Connection*           Connection()                  = 0;
   virtual void                         ForceDetatch()                = 0;
   virtual std::vector<CallstackEntry>& Callstack()                   = 0;
   virtual const CallstackEntry*        CurrentCallFrame()            = 0;
   virtual int                          CurrentCallFrameIndex() const = 0;
   using ScriptInfo                                          = WsfXIO_RequestScriptDataResponsePkt::ScriptDetails;
   virtual ScriptInfo* FindScriptInfo(int64_t aScriptHandle) = 0;
   virtual void        SelectStackFrame(int aFrameIndex)     = 0;
   virtual bool        IsAtBreak() const                     = 0;
   virtual int         BreakSequenceNumber()                 = 0;
};

} // namespace wizard

#endif
