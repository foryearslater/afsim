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
#ifndef DIALOG_BUILDER_RESULTS_DIALOG_HPP
#define DIALOG_BUILDER_RESULTS_DIALOG_HPP

#include <QDialog>

class QPoint;
class QString;
class QVBoxLayout;

namespace WkDialogBuilder
{
class ResultsDialog : public QDialog
{
public:
   //! Returns a reference to a "newed" ResultsDialog.
   //! The dialog has been set to delete itself when closed.
   //! The caller is responsible for calling .show() on the returned value.
   //! Leave aPlatformName empty if there is none.
   static ResultsDialog& MakeResultsDialog(QWidget* aParent, const QString& aScriptName, const QString& aPlatformName);
   ResultsDialog()                     = delete;
   ResultsDialog(const ResultsDialog&) = delete;
   ResultsDialog& operator=(const ResultsDialog&) = delete;

   ~ResultsDialog() override = default;

   void AddReturnValue(const QString& aValue);

   //! If aValid && (aPoint is in the screen), uses aPoint.
   //! Else, places at center of screen and sets aPoint and aValid.
   //! \note Calling object keeps a pointer to aPoint. Make sure the point passed outlives the dialog.
   void SetPosition(QPoint* aPoint, bool* aValid = nullptr);

protected:
   void moveEvent(QMoveEvent* aEvent) override;

private:
   // Leave aPlatformName empty if there is none.
   ResultsDialog(QWidget* aParent, const QString& aScriptName, const QString& aPlatformName);

   //! Initializes the dialog with layouts and boiler plate text.
   void Initialize(const QString& aScriptName, const QString& aPlatformName);

   QPoint DefaultPoint() const;

   QVBoxLayout* mLayout = nullptr;
   QPoint*      mPoint  = nullptr;
};
} // namespace WkDialogBuilder

#endif
