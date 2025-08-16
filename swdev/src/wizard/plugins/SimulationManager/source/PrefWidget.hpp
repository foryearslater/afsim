// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PREFWIDGET_HPP
#define PREFWIDGET_HPP

#include "PrefObject.hpp"
#include "WkfPrefWidget.hpp"

class QRadioButton;
#include <QTableWidgetItem>

#include "ui_SimulationManagerPrefWidget.h"

namespace SimulationManager
{
class SimItem : public QTableWidgetItem
{
public:
   SimItem(QTableWidget*      aParent,
           const QString&     aName,
           const QString&     aPath,
           const QString&     aVersion,
           const QString&     aExeName,
           const QString&     aDate,
           const QString&     aProfilingOutput,
           const QString&     aCliArgs,
           bool               aBuiltIn,
           bool               aGood,
           ut::optional<bool> profiling);
   void SetPath(const QString& aPath) { setData(ePATH, aPath); }
   void SetName(const QString& aName) { setData(eNAME, aName); }
   void SetVersion(const QString& aVersion) { setData(eVERSION, aVersion); }
   void SetBuiltIn(const bool& aBuiltIn) { setData(eBUILTIN, aBuiltIn); }
   void SetExeName(const QString& aExeName) { setData(eEXENAME, aExeName); }
   void SetDateModified(const QString& aDate) { setData(eDATEMODIFIED, aDate); }
   void SetProfilingOutput(const QString& aPath) { setData(ePROFILINGOUT, aPath); }
   void SetExtraCLIArgs(const QString& aArgs) { setData(eCLIARGS, aArgs); }
   void SetSupportsProfiling(const ut::optional<bool> aProfiling)
   {
      setData(ePROFILING, aProfiling.has_value() ? *aProfiling : QVariant());
   }
   QRadioButton* GetRadioButton() const { return mRadioButtonPtr; }

   QString            Path() const { return data(ePATH).toString(); }
   QString            Name() const { return data(eNAME).toString(); }
   QString            Version() const { return data(eVERSION).toString(); }
   bool               BuiltIn() const { return data(eBUILTIN).toBool(); }
   QString            ExeName() const { return data(eEXENAME).toString(); }
   QString            DateModified() const { return data(eDATEMODIFIED).toString(); }
   QString            ProfilingOutput() const { return data(ePROFILINGOUT).toString(); }
   QString            ExtraCLIArgs() const { return data(eCLIARGS).toString(); }
   ut::optional<bool> SupportsProfiling() const
   {
      return data(ePROFILING).isValid() ? data(ePROFILING).toBool() : ut::optional<bool>();
   }

   enum Roles
   {
      ePATH         = Qt::UserRole,
      eNAME         = Qt::UserRole + 1,
      eVERSION      = Qt::UserRole + 2,
      eBUILTIN      = Qt::UserRole + 3,
      eEXENAME      = Qt::UserRole + 4,
      eDATEMODIFIED = Qt::UserRole + 5,
      ePROFILINGOUT = Qt::UserRole + 6,
      eCLIARGS      = Qt::UserRole + 7,
      ePROFILING    = Qt::UserRole + 8,
   };

private:
   QRadioButton* mRadioButtonPtr;
};

class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
   Q_OBJECT
public:
   PrefWidget(QWidget* parent = nullptr);
   ~PrefWidget() override = default;

public slots:
   void DetailsItemDoubleClicked(QTableWidgetItem* item);

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;
   void Add();
   void Remove();
   void ItemChanged();
   void PopulateAppDetails(SimItem* aItem);
   void CellValueChanged(int row, int col);


   bool mItemRemoveFlag;

   Ui::SimulationManagerPrefWidget mUi;
};
} // namespace SimulationManager

#endif
