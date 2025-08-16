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

#ifndef VERTICALCOVERAGEOPTIONSDIALOG_HPP
#define VERTICALCOVERAGEOPTIONSDIALOG_HPP

#include "PlotOptionsDialog.hpp"

class QTreeWidgetItem;

namespace SPLAT
{
class VerticalCoverageOptionsDialog : public PlotOptionsDialog
{
public:
   VerticalCoverageOptionsDialog(QWidget* aParent = nullptr);
   ~VerticalCoverageOptionsDialog() override = default;

   void           WriteData(std::ostream& aStream);
   const QString& GetOutputFilename() { return mOutputFilename; }

private:
   void Build();

   QString mOutputFilename;
};
} // namespace SPLAT
#endif
