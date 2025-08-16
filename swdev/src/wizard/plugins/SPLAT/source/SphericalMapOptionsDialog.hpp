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

#ifndef SPHERICALMAPOPTIONSDIALOG_HPP
#define SPHERICALMAPOPTIONSDIALOG_HPP

#include "PlotOptionsDialog.hpp"

class QTreeWidgetItem;

namespace SPLAT
{
class SphericalMapOptionsDialog : public PlotOptionsDialog
{
public:
   SphericalMapOptionsDialog(QWidget* aParent = nullptr);

   void           WriteData(std::ostream& aStream);
   const QString& GetGnuplotFilename() { return mGnuplotFilename; }

private:
   void    Build();
   QString mGnuplotFilename;
};
} // namespace SPLAT
#endif
