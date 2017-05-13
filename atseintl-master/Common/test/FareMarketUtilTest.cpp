//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <boost/assign/std/vector.hpp>
#include <gtest/gtest.h>
#include <vector>

#include "test/include/GtestHelperMacros.h"
#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"

namespace tse
{

class FareMarketUtilTest : public ::testing::Test
{
protected:
public:
  void SetUp() {}

  void TearDown() {}
};

TEST_F(FareMarketUtilTest, testCollectOwningItinsOne)
{
  using namespace boost::assign;

  FareMarket fm1, fm2, fm3, fm4;
  Itin itin1, itin2;
  itin1.fareMarket() += &fm1, &fm2;
  itin2.fareMarket() += &fm3, &fm4;
  std::vector<Itin*> collection, result;
  collection += &itin1, &itin2;
  result = FareMarketUtil::collectOwningItins(fm2, collection);
  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(&itin1, result[0]);
}

TEST_F(FareMarketUtilTest, testCollectOwningItinsTwo)
{
  using namespace boost::assign;

  FareMarket fm1, fm2, fm3, fm4;
  Itin itin1, itin2;
  itin1.fareMarket() += &fm1, &fm3, &fm2;
  itin2.fareMarket() += &fm3, &fm4;
  std::vector<Itin*> collection, result;
  collection += &itin1, &itin2;
  result = FareMarketUtil::collectOwningItins(fm3, collection);
  ASSERT_EQ(2u, result.size());
  ASSERT_EQ(&itin1, result[0]);
  ASSERT_EQ(&itin2, result[1]);
}

TEST_F(FareMarketUtilTest, testCheckAvailability_Pass)
{
  using namespace boost::assign;

  BookingCode bkcA("A"), bkcB("B");
  ClassOfService cosA, cosB;
  cosA.cabin().setEconomyClass();
  cosA.bookingCode() = bkcA;
  cosA.numSeats() = 3;
  cosB.cabin().setEconomyClass();
  cosB.bookingCode() = bkcB;
  cosB.numSeats() = 1;
  std::vector<ClassOfService*> cosVec;
  cosVec += &cosA, &cosB;

  AirSeg airSeg1, airSeg2;
  FareMarket fm;
  fm.travelSeg() += &airSeg1, &airSeg2;
  fm.classOfServiceVec() += &cosVec, &cosVec;

  ASSERT_TRUE(FareMarketUtil::checkAvailability(fm, bkcA, 3));
}

TEST_F(FareMarketUtilTest, testCheckAvailability_NotAvail)
{
  using namespace boost::assign;

  BookingCode bkc("A");
  ClassOfService cos1, cos2;
  cos1.cabin().setEconomyClass();
  cos1.bookingCode() = bkc;
  cos1.numSeats() = 3;
  cos2.cabin().setEconomyClass();
  cos2.bookingCode() = bkc;
  cos2.numSeats() = 1;
  std::vector<ClassOfService*> cosVec1, cosVec2;
  cosVec1 += &cos1;
  cosVec2 += &cos2;

  AirSeg airSeg1, airSeg2;
  FareMarket fm;
  fm.travelSeg() += &airSeg1, &airSeg2;
  fm.classOfServiceVec() += &cosVec1, &cosVec2;

  ASSERT_FALSE(FareMarketUtil::checkAvailability(fm, bkc, 2));
}

TEST_F(FareMarketUtilTest, testCheckAvailability_NullVector)
{
  using namespace boost::assign;

  BookingCode bkc = "A";
  ClassOfService cos;
  cos.cabin().setEconomyClass();
  cos.bookingCode() = bkc;
  cos.numSeats() = 3;
  std::vector<ClassOfService*> cosVec;
  cosVec += &cos;
  AirSeg airSeg1, airSeg2;
  FareMarket fm;
  fm.travelSeg() += &airSeg1, &airSeg2;
  fm.classOfServiceVec() += &cosVec, static_cast<std::vector<ClassOfService*>*>(0);

  ASSERT_FALSE(FareMarketUtil::checkAvailability(fm, bkc, 1));
}
}
