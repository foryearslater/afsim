// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ImportWorker.hpp"

namespace CrdImporter
{
void ImportWorker::DoConversion(const QString& aParam)
{
   QString result = "Import operation completed normally.";
   mImporter->ParseCrdFiles();
   emit ResultReady(result);
}
} // namespace CrdImporter
