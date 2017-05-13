#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"

using namespace tse;

class NationFranceFlagTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NationFranceFlagTest);
  CPPUNIT_TEST(testNationFranceFlagInFare);
  CPPUNIT_TEST(testNationFranceFlagInPaxTypeFare);
  CPPUNIT_TEST(testCloneInFare);
  CPPUNIT_TEST(testCloneInPaxTypeFare);
  CPPUNIT_TEST_SUITE_END();

  PaxTypeFare* _paxTypeFare;
  Fare* _fare;

public:
  void setUp()
  {
    _paxTypeFare = new PaxTypeFare();
    _fare = new Fare();
  }

  void tearDown()
  {
    delete _paxTypeFare;
    delete _fare;
  }

  void testNationFranceFlagInFare()
  {
    // defaults must be false
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Gr());

    _fare->setNationFRInCat15();
    _fare->setNationFRInCat15Fn();
    _fare->setNationFRInCat15Fr();
    _fare->setNationFRInCat15Gr();

    CPPUNIT_ASSERT(_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Gr());

    _fare->setNationFRInCat15(false);
    _fare->setNationFRInCat15Fn(false);
    _fare->setNationFRInCat15Fr(false);
    _fare->setNationFRInCat15Gr(false);

    CPPUNIT_ASSERT(!_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Gr());

    _fare->setNationFRInCat15(true);
    _fare->setNationFRInCat15Fn(true);
    _fare->setNationFRInCat15Fr(true);
    _fare->setNationFRInCat15Gr(true);

    CPPUNIT_ASSERT(_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Gr());

    _fare->setNationFRInCat15(true);
    _fare->setNationFRInCat15Fn(false);
    _fare->setNationFRInCat15Fr(true);
    _fare->setNationFRInCat15Gr(false);

    CPPUNIT_ASSERT(_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Gr());

    _fare->setNationFRInCat15(false);
    _fare->setNationFRInCat15Fn(true);
    _fare->setNationFRInCat15Fr(false);
    _fare->setNationFRInCat15Gr(true);

    CPPUNIT_ASSERT(!_fare->isNationFRInCat15());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_fare->isNationFRInCat15Fr());
    CPPUNIT_ASSERT(_fare->isNationFRInCat15Gr());
  }

  void testNationFranceFlagInPaxTypeFare()
  {
    // default must be false
    CPPUNIT_ASSERT(!_paxTypeFare->isNationFRInCat35());

    _paxTypeFare->setNationFRInCat35();
    CPPUNIT_ASSERT(_paxTypeFare->isNationFRInCat35());

    _paxTypeFare->setNationFRInCat35(false);
    CPPUNIT_ASSERT(!_paxTypeFare->isNationFRInCat35());

    _paxTypeFare->setNationFRInCat35(true);
    CPPUNIT_ASSERT(_paxTypeFare->isNationFRInCat35());
  }

  void testCloneInFare()
  {
    PricingTrx trx;
    Fare f;

    _fare->setNationFRInCat15();
    _fare->setNationFRInCat15Fn();
    _fare->setNationFRInCat15Fr();
    _fare->setNationFRInCat15Gr();

    _fare->clone(trx.dataHandle(), f);

    CPPUNIT_ASSERT(f.isNationFRInCat15());
    CPPUNIT_ASSERT(f.isNationFRInCat15Fn());
    CPPUNIT_ASSERT(f.isNationFRInCat15Fr());
    CPPUNIT_ASSERT(f.isNationFRInCat15Gr());

    _fare->setNationFRInCat15(false);
    _fare->setNationFRInCat15Fn(false);
    _fare->setNationFRInCat15Fr(false);
    _fare->setNationFRInCat15Gr(false);

    _fare->clone(trx.dataHandle(), f);

    CPPUNIT_ASSERT(!f.isNationFRInCat15());
    CPPUNIT_ASSERT(!f.isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!f.isNationFRInCat15Fr());
    CPPUNIT_ASSERT(!f.isNationFRInCat15Gr());

    _fare->setNationFRInCat15(true);
    _fare->setNationFRInCat15Fn(true);
    _fare->setNationFRInCat15Fr(true);
    _fare->setNationFRInCat15Gr(true);

    _fare->clone(trx.dataHandle(), f);

    CPPUNIT_ASSERT(f.isNationFRInCat15());
    CPPUNIT_ASSERT(f.isNationFRInCat15Fn());
    CPPUNIT_ASSERT(f.isNationFRInCat15Fr());
    CPPUNIT_ASSERT(f.isNationFRInCat15Gr());

    _fare->setNationFRInCat15(false);
    _fare->setNationFRInCat15Fn(true);
    _fare->setNationFRInCat15Fr(false);
    _fare->setNationFRInCat15Gr(true);

    _fare->clone(trx.dataHandle(), f);

    CPPUNIT_ASSERT(!f.isNationFRInCat15());
    CPPUNIT_ASSERT(f.isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!f.isNationFRInCat15Fr());
    CPPUNIT_ASSERT(f.isNationFRInCat15Gr());
  }

  void testCloneInPaxTypeFare()
  {
    PricingTrx trx;
    PaxTypeFare ptf;

    _paxTypeFare->setNationFRInCat35();
    _paxTypeFare->clone(trx.dataHandle(), ptf);
    CPPUNIT_ASSERT(ptf.isNationFRInCat35());

    _paxTypeFare->setNationFRInCat35(false);
    _paxTypeFare->clone(trx.dataHandle(), ptf);
    CPPUNIT_ASSERT(!_paxTypeFare->isNationFRInCat35());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NationFranceFlagTest);
