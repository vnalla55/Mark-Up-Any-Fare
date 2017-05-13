#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeBucket.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/PricingTrx.h"

namespace tse
{

using boost::assign::operator+=;

class PaxTypeBucketTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeBucketTest);
  CPPUNIT_TEST(testConstrutor);
  CPPUNIT_TEST(testSetMarketCurrencyPresent_Empty);
  CPPUNIT_TEST(testSetMarketCurrencyPresent_HasMixed);
  CPPUNIT_TEST(testSetMarketCurrencyPresent_HasNotMixed);
  CPPUNIT_TEST_SUITE_END();

public:
  PaxTypeFare* createFare(CurrencyCode curr)
  {
    FareInfo* fi = _memH.create<FareInfo>();
    fi->currency() = curr;
    Fare* f = _memH.create<Fare>();
    f->setFareInfo(fi);
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    ptf->setFare(f);
    return ptf;
  }

  void testConstrutor()
  {
    PaxTypeBucket ptc;
    CPPUNIT_ASSERT(!ptc.isMarketCurrencyPresent());
  }

  void testSetMarketCurrencyPresent_Empty()
  {
    PaxTypeBucket ptc;
    ptc.setMarketCurrencyPresent(false);
    CPPUNIT_ASSERT(!ptc.isMarketCurrencyPresent());
  }

  void testSetMarketCurrencyPresent_HasMixed()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare(USD), createFare(CAD), createFare(USD);
    ptc.setMarketCurrencyPresent(true);

    CPPUNIT_ASSERT(ptc.isMarketCurrencyPresent());
  }

  void testSetMarketCurrencyPresent_HasNotMixed()
  {
    PaxTypeBucket ptc;
    ptc.paxTypeFare() += createFare(USD), createFare(USD), createFare(USD);
    ptc.setMarketCurrencyPresent(false);

    CPPUNIT_ASSERT(!ptc.isMarketCurrencyPresent());
  }

protected:
  TestMemHandle _memH;
  static const CurrencyCode USD, CAD;
  PricingTrx _trx;
};

const CurrencyCode PaxTypeBucketTest::USD = "USD", PaxTypeBucketTest::CAD = "CAD";

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeBucketTest);

} // tse
