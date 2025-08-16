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
#ifndef PROJECTCOMPONENT_HPP
#define PROJECTCOMPONENT_HPP

#include "UtQtXmlReader.hpp"
#include "UtQtXmlWriter.hpp"
#include "ViExport.hpp"

namespace wizard
{
class VI_EXPORT ProjectComponent
{
public:
   virtual ~ProjectComponent() = default;
   virtual void WriteXML(UtQtXmlWriter& aWriter) const {}
   virtual bool ReadXML(UtQtXmlElementReader& aReader) { return false; }
   virtual void ProjectDirectoryChanged() {}
};
} // namespace wizard
#endif
