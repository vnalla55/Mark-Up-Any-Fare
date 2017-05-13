//---------------------------------------------------------------------------
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

#include "Taxes/LegacyTaxes/ProrateCalculator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace
{
LocCode LAX = "LAX";
LocCode DFW = "DFW";
LocCode FRA = "FRA";
LocCode ORD = "ORD";
}

using testing::Return;
using testing::StrictMock;

struct GetMileage
{
  MOCK_CONST_METHOD2(get, uint32_t(const LocCode&, const LocCode&));

  uint32_t operator()(const Loc& origin,
                      const Loc& destination,
                      const GlobalDirection gd,
                      const DateTime&,
                      DataHandle&) const
  {
    return get(origin.loc(), destination.loc());
  }
};

class ProrateCalculatorTest : public testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _getMileage = _memHandle.create<StrictMock<GetMileage> >();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _dateTime = new DateTime(DateTime::openDate());
  }

  void TearDown()
  {
    _memHandle.clear();
    std::vector<TravelSeg*>().swap(_travelSegs);
    delete _dateTime;
  }

protected:
  TestMemHandle _memHandle;
  ProrateCalculator* _calculator;
  StrictMock<GetMileage>* _getMileage;
  PricingTrx* _trx;
  DateTime* _dateTime;
  std::vector<TravelSeg*> _travelSegs;

  void initCalculator()
  {
    _calculator = _memHandle.create<ProrateCalculator>(
        *_trx, *_dateTime, _travelSegs, boost::ref(*_getMileage));
  }

  AirSeg* createAir(const LocCode& origin, const LocCode& destination, const CarrierCode& carrier)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = createLoc(origin);
    seg->destination() = createLoc(destination);
    seg->carrier() = carrier;
    return seg;
  }

  ArunkSeg* createArunk(const LocCode& origin, const LocCode& destination)
  {
    ArunkSeg* seg = _memHandle.create<ArunkSeg>();
    seg->origin() = createLoc(origin);
    seg->destination() = createLoc(destination);
    return seg;
  }

  Loc* createLoc(const LocCode& code)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = code;
    return loc;
  }
};

TEST_F(ProrateCalculatorTest, TestProratedAmount1Segment)
{
  _travelSegs.push_back(createAir(LAX, DFW, "AA"));
  initCalculator();

  EXPECT_CALL(*_getMileage, get(LAX, DFW)).WillOnce(Return(100));

  ASSERT_EQ(100.0, _calculator->getProratedAmount(0, 0, 100.0));
}

TEST_F(ProrateCalculatorTest, TestProratedAmount2Segments)
{
  _travelSegs.push_back(createAir(LAX, DFW, "AA"));
  _travelSegs.push_back(createAir(DFW, FRA, "AA"));
  initCalculator();

  EXPECT_CALL(*_getMileage, get(LAX, DFW)).WillOnce(Return(100));
  EXPECT_CALL(*_getMileage, get(DFW, FRA)).WillOnce(Return(400));

  ASSERT_EQ(20.0, _calculator->getProratedAmount(0, 0, 100.0));
}

TEST_F(ProrateCalculatorTest, TestProratedAmount3SegmentsWithArunk)
{
  _travelSegs.push_back(createAir(LAX, DFW, "AA"));
  _travelSegs.push_back(createArunk(DFW, ORD));
  _travelSegs.push_back(createAir(ORD, FRA, "AA"));
  initCalculator();

  EXPECT_CALL(*_getMileage, get(LAX, DFW)).WillOnce(Return(100));
  EXPECT_CALL(*_getMileage, get(DFW, ORD)).Times(0);
  EXPECT_CALL(*_getMileage, get(ORD, FRA)).WillOnce(Return(400));

  ASSERT_EQ(20.0, _calculator->getProratedAmount(0, 1, 100.0));
}
}
