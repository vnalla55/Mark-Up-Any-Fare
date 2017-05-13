#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include <string>
#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace tse
{

class TaxLocIteratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxLocIteratorTest);
  CPPUNIT_TEST(iterate_2segs);
  CPPUNIT_TEST(iterate_3segs_withStop);
  CPPUNIT_TEST(iterate_3segs_hidden);
  CPPUNIT_TEST(iterate_3segs_hiddenSkipHidden);
  CPPUNIT_TEST_SUITE_END();

  std::string xmlPath;
  Itin itin;
  PricingTrx* trx;
  TaxResponse* taxResponse;
  FarePath* farePath;

public:
  void setUp()
  {
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    itin.travelSeg().clear();
    trx = new PricingTrx();
    taxResponse = new TaxResponse();
    farePath = new FarePath();
    taxResponse->farePath() = farePath;
    farePath->itin() = &itin;
    farePath->pricingUnit().clear();
  }

  void tearDown()
  {
    delete farePath;
    delete taxResponse;
    delete trx;
  }

  void iterate_2segs()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml", true));

    PricingUnit pricingUnit;
    farePath->pricingUnit().push_back(&pricingUnit);
    FareUsage fareUsage;
    pricingUnit.fareUsage().push_back(&fareUsage);
    fareUsage.travelSeg().push_back(itin.travelSeg()[0]);
    FareUsage fareUsage2;
    pricingUnit.fareUsage().push_back(&fareUsage2);
    fareUsage2.travelSeg().push_back(itin.travelSeg()[1]);

    TaxLocIterator li;
    li.initialize(*farePath);

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), false);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.next();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), false);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), true);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), true);

    li.next();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), false);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL(li.prevSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), true);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.previous();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), false);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), true);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), true);

    li.previous();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), false);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.toBack();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), false);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL(li.prevSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), true);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);
  }

  void iterate_3segs_withStop()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml", true));
    itin.travelSeg().push_back(
        TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1_forcedStop.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml", true));

    TaxLocIterator li;
    li.initialize(*farePath);

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), false);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.next();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.prevSeg(), itin.travelSeg()[0]);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), false);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.next();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), true);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 2);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL(li.prevSeg(), itin.travelSeg()[1]);
    CPPUNIT_ASSERT_EQUAL(li.nextSeg(), itin.travelSeg()[2]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);

    li.next();

    CPPUNIT_ASSERT_EQUAL(li.hasPrevious(), true);
    CPPUNIT_ASSERT_EQUAL(li.hasNext(), false);
    CPPUNIT_ASSERT_EQUAL((int)li.prevSegNo(), 2);
    CPPUNIT_ASSERT_EQUAL(li.prevSeg(), itin.travelSeg()[2]);
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL(li.isStop(), true);
    CPPUNIT_ASSERT_EQUAL(li.isFareBreak(), false);
    CPPUNIT_ASSERT_EQUAL(li.isTurnAround(), false);
  }

  void iterate_3segs_hidden()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml", true));

    Loc loc;

    itin.travelSeg()[1]->hiddenStops().push_back(&loc);

    TaxLocIterator li;
    li.initialize(*farePath);
    li.next();
    li.next();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    li.next();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 2);
    li.previous();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 1);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    li.previous();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    li.previous();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 0);
  }

  void iterate_3segs_hiddenSkipHidden()
  {
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml", true));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml", true));

    Loc loc;

    itin.travelSeg()[1]->hiddenStops().push_back(&loc);

    TaxLocIterator li;
    li.setSkipHidden(true);
    li.initialize(*farePath);
    li.next();
    li.next();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 2);
    li.next();
    CPPUNIT_ASSERT(!li.hasNext());
    li.previous();
    li.previous();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 1);
    li.previous();
    CPPUNIT_ASSERT_EQUAL((int)li.subSegNo(), 0);
    CPPUNIT_ASSERT_EQUAL((int)li.nextSegNo(), 0);
    CPPUNIT_ASSERT(!li.hasPrevious());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxLocIteratorTest);
};
