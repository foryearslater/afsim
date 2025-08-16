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

#ifndef DOCUMENTATIONDIALOG_HPP
#define DOCUMENTATIONDIALOG_HPP

#include "WkfDocumentationDialog.hpp"

namespace wizard
{
class DocumentationDialog : public wkf::DocumentationDialog
{
public:
   DocumentationDialog(wkf::MainWindow* parent, Qt::WindowFlags f = Qt::WindowFlags());
   ~DocumentationDialog() override = default;

protected:
   void PopulateDialog() override;
};
} // namespace wizard

#endif
