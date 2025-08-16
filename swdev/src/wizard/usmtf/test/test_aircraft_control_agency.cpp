// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <string>

#include <gtest/gtest.h>

#include "AircraftControlAgency.hpp"
#include "Field.hpp"

class AircraftControlAgencyParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

class AircraftControlAgencyParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

TEST_P(AircraftControlAgencyParameterizedFixtureTestsValid, WorksForAllAgencies)
{
   std::string                  agency = GetParam();
   usmtf::AircraftControlAgency validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{agency}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(agency, validator.GetAgency());
}

TEST_P(AircraftControlAgencyParameterizedFixtureTestsInvalid, FailsForInvalidAgenciesAllAgencies)
{
   std::string                  agency = GetParam();
   usmtf::AircraftControlAgency validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{agency}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetAgency());
   validator.LogErrors(std::cout);
}

INSTANTIATE_TEST_CASE_P(AircraftControlAgencyTestsValid,
                        AircraftControlAgencyParameterizedFixtureTestsValid,
                        ::testing::Values("AOC",
                                          "ASOC",
                                          "ABC",
                                          "AWAC",
                                          "ASCA",
                                          "CCT",
                                          "CAOC",
                                          "CRC",
                                          "CRE",
                                          "CRP",
                                          "DASC",
                                          "DASA",
                                          "EWC",
                                          "FACP",
                                          "FAC",
                                          "FACA",
                                          "GCI",
                                          "HST",
                                          "JAOC",
                                          "ITG",
                                          "MATC",
                                          "MMT",
                                          "OTR",
                                          "TACC",
                                          "TACP",
                                          "TACA",
                                          "TADC",
                                          "TAOC"), );

INSTANTIATE_TEST_CASE_P(AircraftControlAgencyTestsInvalid,
                        AircraftControlAgencyParameterizedFixtureTestsInvalid,
                        ::testing::Values("Not", "Valid", "Agencies", "at", "all"), );
