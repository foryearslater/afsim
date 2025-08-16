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

#include "ContextDocumentationControl.hpp"

#include "BrowserWidget.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

wizard::ContextDocumentationControl::ContextDocumentationControl()
{
   setObjectName("ContextDocumentationControl");
   setWindowTitle("Context Help");
   setWidget(new BrowserWidget(wkfEnv.GetMainWindow()));
   mBrowserPtr = static_cast<BrowserWidget*>(widget());
}
