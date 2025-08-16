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

#ifndef SPACETOOLSUTIL_HPP
#define SPACETOOLSUTIL_HPP

#include <QJsonDocument>
#include <QString>

namespace SpaceTools
{
namespace Util
{
//! Get the database located at the given path
//!
//! @param aPath The path where the database is located
//! @returns The database at the given path. Returns null if database doesn't exist or isn't a json file
QJsonDocument GetDatabase(const QString& aPath);
} // namespace Util
}; // namespace SpaceTools
#endif
