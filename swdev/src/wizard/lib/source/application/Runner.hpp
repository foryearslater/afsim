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

#ifndef WIZARDRUNNER_HPP
#define WIZARDRUNNER_HPP

#include <memory>

#include "UtPath.hpp"

class QApplication;

namespace wizard
{

class Runner
{
public:
   Runner(int&        argc,
          char*       argv[],
          const char* aCompanyStr,
          const char* aCompanyDomainStr,
          const char* aProductNameStr,
          const char* aProductVersionStr);
   Runner(const Runner&) = delete;
   Runner(Runner&&)      = default;
   Runner& operator=(const Runner&) = delete;
   Runner& operator=(Runner&&) = default;
   void    run();
   ~Runner();

private:
   std::unique_ptr<QApplication> mApp;
   UtPath                        mBinDir;
   std::string                   mRccPath;
};

} // namespace wizard

#endif
