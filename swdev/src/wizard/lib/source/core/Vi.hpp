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

#ifndef VI_HPP
#define VI_HPP

namespace Vi
{
enum EditMode
{
   // Text mode is active by default until a change
   cEDIT_MODE_TEXT,
   cEDIT_MODE_VISUAL,
   // While switching to text mode, both text and object representations are read-only
   cEDIT_MODE_SWITCHING_TO_TEXT
};
}

#endif
