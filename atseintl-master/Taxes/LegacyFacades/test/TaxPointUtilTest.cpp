// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyFacades/TaxPointUtil.h"

#include "test/include/CppUnitHelperMacros.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <memory>

namespace tse
{

class TaxPointUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointUtilTest);
  CPPUNIT_TEST(testSetTravelSegIndices);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {

    _itin.reset(new Itin);
    for (uint16_t i = 0; i < 4; ++i)
    {
      _airSegs.push_back(new AirSeg);
    }

    _hidden.reset(new Loc);

    for (uint16_t i = 0; i < 4; ++i)
    {
      _itin->travelSeg().push_back(&_airSegs[i]);
    }
  }

  void testSetTravelSegIndices()
  {
    _airSegs[2].hiddenStops().push_back(_hidden.get());
    uint16_t start = 0xFFFF;
    uint16_t end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices( 0, 3, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(1), end);

    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(0, 5, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), end);

    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(0, 9, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), end);

    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(4, 9, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(2), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), end);

    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(6, 9, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0xFFFF), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), end);

    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(8, 9, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), end);

    // reversed
    start = 0xFFFF;
    end = 0xFFFF;
    TaxPointUtil::setTravelSegIndices(9, 0, *_itin, start, end);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), start);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), end);
  }

private:
  std::unique_ptr<Itin> _itin;
  boost::ptr_vector<AirSeg> _airSegs;
  std::unique_ptr<Loc> _hidden;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointUtilTest);
}
