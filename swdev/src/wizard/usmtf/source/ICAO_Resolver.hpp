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

#ifndef ICAORESOLVER_HPP
#define ICAORESOLVER_HPP

#include "usmtf_export.h"

#include <string>
#include <unordered_map>

namespace usmtf
{

/*!
   Class for Looking up code in the ICAO reference file and returning lat/long.
   ICAO Names are simply common names that have associated global positioning.
*/
class USMTF_EXPORT ICAO_Resolver
{
public:
   ICAO_Resolver();
   //! Looks up an ICAO code and returns formatted position data.
   std::string LookUpCode(const std::string& aIcaoCode) const noexcept;
   //! Access the ICAO reference file. Multiple calls used cached data structure
   //! unless force flag is set to true.
   void LoadCodeMap(bool aForceReload = false) noexcept;

private:
   std::unordered_map<std::string, std::string> ReadConf() const noexcept;
   //! Its static for performance, all instances of ICAO Resolver
   //! can use the cached code. This is needed because the primary
   //! use for this will be any set who needs ICAO name lookup. This could be
   //! hundreds of sets in 1 USMTF Message.
   static std::unordered_map<std::string, std::string> mCodes;
   static std::unordered_map<std::string, std::string> mConf;
   static bool                                         mHasLoadedCodes;
   static bool                                         mHasLoadedConf;
};
} // namespace usmtf

#endif
