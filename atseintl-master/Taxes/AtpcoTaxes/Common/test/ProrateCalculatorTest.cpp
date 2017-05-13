// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/ProrateCalculator.h"

#include <gmock/gmock.h>

#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tax
{
namespace
{
class MileageGetterMock : public MileageGetter
{
public:
  MOCK_CONST_METHOD2(getSingleDistance, type::Miles(const type::Index&, const type::Index&));
  MOCK_CONST_METHOD2(getSingleGlobalDir,
                     type::GlobalDirection(const type::Index&, const type::Index&));
};
}

using testing::StrictMock;
using testing::Return;

class ProrateCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ProrateCalculatorTest);
  CPPUNIT_TEST(testGetProratedAmount_InsideRange);
  CPPUNIT_TEST(testGetProratedAmount_LeftOutside);
  CPPUNIT_TEST(testGetProratedAmount_RightOutside);
  CPPUNIT_TEST(testGetProratedAmount_ExactMatch);
  CPPUNIT_TEST(testGetProratedAmount_InsideRange_WithHidden);
  CPPUNIT_TEST(testGetProratedAmount_LeftOutside_WithHidden);
  CPPUNIT_TEST(testGetProratedAmount_RightOutside_WithHidden);
  CPPUNIT_TEST(testGetProratedAmount_ExactMatch_WithHidden);
  CPPUNIT_TEST(testGetProratedAmount_InsideRange_WithHidden_SkipHidden);
  CPPUNIT_TEST(testGetProratedAmount_LeftOutside_WithHidden_SkipHidden);
  CPPUNIT_TEST(testGetProratedAmount_RightOutside_WithHidden_SkipHidden);
  CPPUNIT_TEST(testGetProratedAmount_ExactMatch_WithHidden_SkipHidden);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp()
  {
    _mileageGetter.reset(new StrictMock<MileageGetterMock>());
    _geoPath.reset(new GeoPath);
    _geoPath->geos().resize(10);
    for (int i = 0; i < 10; ++i)
    {
      _geoPath->geos()[i].id() = i;
    }
  }

  void testGetProratedAmount_InsideRange()
  {
    Range baseRange(0, 3), taxRange(0, 1);

    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).WillOnce(Return(300));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(25), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath));
  }

  void testGetProratedAmount_LeftOutside()
  {
    Range baseRange(2, 5), taxRange(0, 3);

    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).WillOnce(Return(300));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(25), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath));
  }

  void testGetProratedAmount_RightOutside()
  {
    Range baseRange(0, 3), taxRange(2, 7);

    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(300));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(75), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath));
  }

  void testGetProratedAmount_ExactMatch()
  {
    Range baseRange(2, 5), taxRange(2, 5);

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(100), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath));
  }

  void testGetProratedAmount_InsideRange_WithHidden()
  {
    Range baseRange(0, 7), taxRange(2, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).WillOnce(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(50), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, false));
  }

  void testGetProratedAmount_LeftOutside_WithHidden()
  {
    Range baseRange(2, 9), taxRange(0, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(8, 9)).WillOnce(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(50), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, false));
  }

  void testGetProratedAmount_RightOutside_WithHidden()
  {
    Range baseRange(0, 7), taxRange(4, 9);

    _geoPath->geos()[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[5].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[6].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 3)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 5)).Times(2).WillRepeatedly(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).Times(2).WillRepeatedly(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(50), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, false));
  }

  void testGetProratedAmount_ExactMatch_WithHidden()
  {
    Range baseRange(2, 5), taxRange(2, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(100), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, false));
  }

  void testGetProratedAmount_InsideRange_WithHidden_SkipHidden()
  {
    Range baseRange(0, 7), taxRange(2, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 1)).WillOnce(Return(100));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 5)).Times(2).WillRepeatedly(Return(200));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 7)).WillOnce(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(50), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, true));
  }

  void testGetProratedAmount_LeftOutside_WithHidden_SkipHidden()
  {
    Range baseRange(2, 9), taxRange(0, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[7].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[8].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(2, 5)).Times(2).WillRepeatedly(Return(300));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(6, 9)).WillOnce(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(75), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, true));
  }

  void testGetProratedAmount_RightOutside_WithHidden_SkipHidden()
  {
    Range baseRange(0, 7), taxRange(4, 9);

    _geoPath->geos()[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[5].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[6].unticketedTransfer() = type::UnticketedTransfer::Yes;
    EXPECT_CALL(*_mileageGetter, getSingleDistance(0, 3)).WillOnce(Return(300));
    EXPECT_CALL(*_mileageGetter, getSingleDistance(4, 7)).Times(2).WillRepeatedly(Return(100));

    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(25), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, true));
  }

  void testGetProratedAmount_ExactMatch_WithHidden_SkipHidden()
  {
    Range baseRange(2, 5), taxRange(2, 5);

    _geoPath->geos()[3].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _geoPath->geos()[4].unticketedTransfer() = type::UnticketedTransfer::Yes;
    _calculator.reset(new ProrateCalculator(*_mileageGetter));
    ASSERT_EQ(type::MoneyAmount(100), _calculator->getProratedAmount(baseRange, taxRange, 100, *_geoPath, true));
  }

private:
  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<StrictMock<MileageGetterMock>> _mileageGetter;
  std::unique_ptr<ProrateCalculator> _calculator;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProrateCalculatorTest);
}
