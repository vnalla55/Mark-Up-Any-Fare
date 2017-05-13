//
// Copyright Sabre 2012-02-06
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "ItinAnalyzer/SoloCarnivalIAUtil.h"
#include "ItinAnalyzer/test/TravelSegmentTestUtil.h"
#include "DataModel/PricingOptions.h"

namespace tse
{

class SoloCarnivalIAUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SoloCarnivalIAUtilTest);
  CPPUNIT_TEST(testFillItinHashesHandlesSimilarItins);
  CPPUNIT_TEST(testAddSubItinInTravelOrder);
  CPPUNIT_TEST_SUITE_END();

public:
  void testFillItinHashesHandlesSimilarItins()
  {
    TravelSeg* s1 = createAirSegment("AAA", "BBB", "CA", "2012-01-01");
    TravelSeg* s2 = createAirSegment("AAA", "BBB", "CA", "2012-01-01");
    TravelSeg* s3 = createAirSegment("AAA", "BBB", "CA", "2012-01-01");
    s1->originalId() = 1;
    s2->originalId() = 2;
    s3->originalId() = 3;

    Itin i1, i2, i3;
    i1.travelSeg().push_back(s1);
    i2.travelSeg().push_back(s2);
    i3.travelSeg().push_back(s3);

    PricingTrx trx;
    PricingOptions options;
    trx.setOptions(&options);
    trx.itin().push_back(&i1);
    i1.addSimilarItin(&i2);
    trx.itin().push_back(&i3);

    SoloCarnivalIAUtil util(trx);
    util.fillItinHashes();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), util.itinHashes().size());
  }

  void testAddSubItinInTravelOrder()
  {
    using namespace boost::assign;
    TravelSeg* s1 = createAirSegment("AAA", "BBB", "CA", "2012-01-01");
    TravelSeg* s2 = createAirSegment("BBB", "CCC", "CA", "2012-01-01");
    TravelSeg* s3 = createAirSegment("CCC", "DDD", "CA", "2012-01-01");

    Itin original;
    original.travelSeg() += s1, s2, s3;

    Itin sub1, sub2, sub3;
    sub1.travelSeg() += s1;
    sub2.travelSeg() += s2;
    sub3.travelSeg() += s3;

    SOLItinGroups::ItinGroup group;
    group += &sub2, &sub3, &sub1;

    SoloCarnivalIAUtil::sortGroupInTravelOrder(group, &original);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), group.size());
    CPPUNIT_ASSERT_EQUAL(&sub1, group[0]);
    CPPUNIT_ASSERT_EQUAL(&sub2, group[1]);
    CPPUNIT_ASSERT_EQUAL(&sub3, group[2]);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SoloCarnivalIAUtilTest);
}
