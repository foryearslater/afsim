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

#ifndef CRDIMPORTWORKER_HPP
#define CRDIMPORTWORKER_HPP

#include <iostream>

#include <QObject>

#include "CrdFileImporter.hpp"

namespace CrdImporter
{
class ImportWorker : public QObject
{
   Q_OBJECT

public:
   ImportWorker()           = default;
   ~ImportWorker() override = default;

   void SetCrdImporter(CrdFileImporter& crdImporter) { mImporter = &crdImporter; }

   void DoConversion(const QString& aParam);

signals:
   void ResultReady(const QString& aResult);

private:
   CrdFileImporter* mImporter{nullptr};
};
} // namespace CrdImporter
#endif
