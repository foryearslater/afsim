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

#ifndef SATELLITEINSERTER_HPP
#define SATELLITEINSERTER_HPP

#include <QPointer>

#include "SatelliteInserterDialog.hpp"
#include "SatelliteInserterPrefWidget.hpp"

namespace SpaceTools
{
//! The Satellite Inserter space tool allows the user to add predefined satellite definitions into
//! scenarios. The user can also add their own databases and platform definitions.
class SatelliteInserter : public QObject
{
   Q_OBJECT

public:
   explicit SatelliteInserter();
   ~SatelliteInserter() override;

   void             ShowDialog();
   void             OnProjectClosed();
   void             OnProjectOpened();
   wkf::PrefWidget* GetPrefWidget() const { return mPrefWidgetPtr; }

private:
   QPointer<PrefWidget>     mPrefWidgetPtr;
   SatelliteInserterDialog* mDialogPtr;
};
} // namespace SpaceTools
#endif // SATELLITEINSERTER_HPP
