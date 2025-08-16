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
#include "LatLon.hpp"
#include "VariantValidator.hpp"
#include "VerifiedTimePoint.hpp"

TEST(TestVariantValidator, WorksAsExpected)
{
   const usmtf::Field                                               f{"141325ZFEB"};
   usmtf::VariantValidator<usmtf::VerifiedTimePoint, usmtf::LatLon> vv{};
   vv.ParseAndValidate(f);
   EXPECT_TRUE(vv.IsValid());
   EXPECT_TRUE(vv.IsMatch<usmtf::VerifiedTimePoint>());
   EXPECT_TRUE(vv.HasMatch());

   // Example usage
   if (usmtf::VerifiedTimePoint* vptr = vv.IsMatch<usmtf::VerifiedTimePoint>())
   {
      EXPECT_EQ(vptr->GetDay(), "14");
      EXPECT_EQ(vptr->GetMonth(), "FEB");
      EXPECT_EQ(vptr->GetYear(), "");
      EXPECT_EQ(vptr->GetTimeZone(), "Z");
      EXPECT_EQ(vptr->GetMinute(), "25");
      EXPECT_EQ(vptr->GetHour(), "13");
   }
}

TEST(TestVariantValidator, WorksAsExpectedWhenErrorConditionHappensFirst)
{
   const usmtf::Field                                               f{"141325ZFEB"};
   usmtf::VariantValidator<usmtf::LatLon, usmtf::VerifiedTimePoint> vv{};
   vv.ParseAndValidate(f);
   EXPECT_TRUE(vv.IsValid());
   EXPECT_TRUE(vv.IsMatch<usmtf::VerifiedTimePoint>());

   // Example usage
   if (usmtf::VerifiedTimePoint* vptr = vv.IsMatch<usmtf::VerifiedTimePoint>())
   {
      EXPECT_EQ(vptr->GetDay(), "14");
      EXPECT_EQ(vptr->GetMonth(), "FEB");
      EXPECT_EQ(vptr->GetYear(), "");
      EXPECT_EQ(vptr->GetTimeZone(), "Z");
      EXPECT_EQ(vptr->GetMinute(), "25");
      EXPECT_EQ(vptr->GetHour(), "13");
   }
}


TEST(TestVariantValidator, AsAClassProp)
{
   class Compose
   {
   public:
      Compose(const usmtf::Field& f)
      {
         if (!one.ParseAndValidate(f))
         {
            // AddErrors!
         }
      }

      const usmtf::VariantValidator<usmtf::VerifiedTimePoint, usmtf::LatLon>& GetOne() { return one; }

   private:
      usmtf::VariantValidator<usmtf::VerifiedTimePoint, usmtf::LatLon> one{};
   };

   const usmtf::Field f{"141325ZFEB"};
   Compose            comp{f}; // This can be though of as a Set, where one member is a variantValidator

   // Example usage
   if (usmtf::VerifiedTimePoint* vptr = comp.GetOne().IsMatch<usmtf::VerifiedTimePoint>())
   {
      if (vptr)
      {
         EXPECT_EQ(vptr->GetDay(), "14");
         EXPECT_EQ(vptr->GetMonth(), "FEB");
         EXPECT_EQ(vptr->GetYear(), "");
         EXPECT_EQ(vptr->GetTimeZone(), "Z");
         EXPECT_EQ(vptr->GetMinute(), "25");
         EXPECT_EQ(vptr->GetHour(), "13");
      }
   }
}

TEST(TestVariantValidator, FailsBothReportsErrors)
{
   const usmtf::Field                                               f{"Nah"};
   usmtf::VariantValidator<usmtf::VerifiedTimePoint, usmtf::LatLon> vv{};
   vv.ParseAndValidate(f);
   EXPECT_FALSE(vv.IsValid());
   usmtf::VerifiedTimePoint* vptr = vv.IsMatch<usmtf::VerifiedTimePoint>();
   EXPECT_FALSE(vv.IsMatch<usmtf::VerifiedTimePoint>());
   EXPECT_EQ(vptr, nullptr);
   EXPECT_FALSE(vv.HasMatch());
   vv.LogErrors(std::cout);
}
