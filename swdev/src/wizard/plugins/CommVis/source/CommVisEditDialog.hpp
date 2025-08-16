// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#ifndef COMMVIS_EDITDIALOG_HPP
#define COMMVIS_EDITDIALOG_HPP
#include <QString>

class UtTextDocumentRange;
class WsfPProxyNode;
namespace WizCommVis
{
class CommVisEditDialog
{
public:
   CommVisEditDialog()                                 = default;
   virtual ~CommVisEditDialog()                        = 0; // Make this an abstract class
   CommVisEditDialog(const CommVisEditDialog& aDialog) = default;

protected:
   size_t GetNumSpaces(std::string aLine) const noexcept;

   void AddNewDevice(const UtTextDocumentRange& aDocRange, const QString& aBlockStartStr, const QString& aBlockEndStr);

   QString GetIndentString(std::string aLine) const;

   virtual const QString GetAttributeString(int aTabSize) const { return QString(); }

   void EditAttribute(const QString& aAttributeStr, const UtTextDocumentRange& aDocRange);

   void AddNewAttribute(const QString& aNewAttributeStr, const WsfPProxyNode& aNode);
};
} // namespace WizCommVis

#endif
