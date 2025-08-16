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

#ifndef VERTICALMAPOPTIONSDIALOG_HPP
#define VERTICALMAPOPTIONSDIALOG_HPP

#include "PlotOptionsDialog.hpp"

class QTreeWidgetItem;

namespace SPLAT
{
class VerticalMapOptionsDialog : public PlotOptionsDialog
{
public:
   VerticalMapOptionsDialog(QWidget* aParent = nullptr);
   ~VerticalMapOptionsDialog() override = default;

   void           WriteData(std::ostream& aStream);
   const QString& GetGnuplotFilename() { return mGnuplotFilename; }

private:
   void Build();

   QString mGnuplotFilename;
};
} // namespace SPLAT
#endif