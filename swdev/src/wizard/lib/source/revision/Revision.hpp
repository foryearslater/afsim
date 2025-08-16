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

#ifndef REVISION_HPP
#define REVISION_HPP

#include <vector>

#include <QDateTime>

#include "ViExport.hpp"

namespace wizard
{
//! A struct that represents a revision in the Change History database.
struct VI_EXPORT Revision
{
   Revision(int aRevNo, int aParentRevNo, const std::string& aWorkingDir, int aTimestamp);

   int                      revNo;
   int                      parentRevNo;
   std::string              workingDir;
   std::vector<std::string> startupFiles;
   QDateTime                time;
};

typedef std::vector<Revision> RevisionList;
} // namespace wizard
#endif // REVISION_HPP
