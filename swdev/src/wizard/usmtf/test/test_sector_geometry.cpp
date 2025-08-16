// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "Field.hpp"
#include "SectorGeometry.hpp"

TEST(SectorGeometryValidatorTests, IsValidForValidSectorGeometry)
{
   const usmtf::Field    f{"058190T45-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_TRUE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "58");
   EXPECT_EQ(sg.GetSectorStopAngle(), "190");
   EXPECT_EQ(sg.GetAngleOrientation(), "T");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "45.000000");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "60.000000");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "NM");
   sg.LogErrors(std::cout);
}


TEST(SectorGeometryValidatorTests, IsInValidForInvalidStartAngleLength)
{
   const usmtf::Field    f{"1000190T45-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}

TEST(SectorGeometryValidatorTests, IsInValidForInvalidStopAngleLength)
{
   const usmtf::Field    f{"1001900T45-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}

TEST(SectorGeometryValidatorTests, IsInValidForInvalidStartAngleRange)
{
   const usmtf::Field    f{"361190T45-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}

TEST(SectorGeometryValidatorTests, IsInValidForInvalidStopAngleRange)
{
   const usmtf::Field    f{"100361T45-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}

TEST(SectorGeometryValidatorTests, IsInValidForMissingOrientation)
{
   const usmtf::Field    f{"100248Z45-60YD"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}

TEST(SectorGeometryValidatorTests, CorrectlyDelegatesRangeCheck)
{
   const usmtf::Field    f{"100248G100000-60NM"};
   usmtf::SectorGeometry sg{};
   sg.ParseAndValidate(f);
   EXPECT_FALSE(sg.IsValid());
   EXPECT_EQ(sg.GetSectorStartAngle(), "");
   EXPECT_EQ(sg.GetSectorStopAngle(), "");
   EXPECT_EQ(sg.GetAngleOrientation(), "");
   EXPECT_EQ(sg.GetRange().GetInnerRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetRadiusValue(), "");
   EXPECT_EQ(sg.GetRange().GetUnitOfMeasure(), "");
   sg.LogErrors(std::cout);
}
