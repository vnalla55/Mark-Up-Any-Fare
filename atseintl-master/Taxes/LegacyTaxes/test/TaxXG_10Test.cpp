#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxXG_10.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/test/TaxLocIteratorMock.h"
#include "DBAccess/TaxCodeReg.h"

namespace tse
{

class TaxXG_10m : public TaxXG_10
{
public:
  TaxLocIteratorMock _locItMock;

  bool isInternationalItin(TaxLocIterator* locIt) { return false; }

  virtual bool validateFromTo(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
  {
    return false;
  }

protected:
  TaxLocIterator* getLocIterator(FarePath& farePath) { return &_locItMock; }
};

class TaxXG_10Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxXG_10Test);
  CPPUNIT_TEST(oneSegmentItin_correctLocations);
  CPPUNIT_TEST(oneSegmentItin_inCorrectLocations1);
  CPPUNIT_TEST(oneSegmentItin_inCorrectLocations2);
  CPPUNIT_TEST(twoSegmentItin_correctLocations);
  CPPUNIT_TEST(twoSegmentItin_stopInCorrectLocations);
  CPPUNIT_TEST(twoSegmentItin_noStopInCorrectLocations);
  CPPUNIT_TEST(twoSegmentItin_hiddenCity);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx* trx;
  TaxResponse* taxResponse;
  TaxCodeReg* taxCodereg;
  uint16_t startIndex, endIndex;

public:
  void setUp()
  {
    trx = new PricingTrx();
    taxResponse = new TaxResponse();
    taxCodereg = new TaxCodeReg();
  }

  void tearDown()
  {
    delete taxCodereg;
    delete taxResponse;
    delete trx;
  }

  void oneSegmentItin_correctLocations()
  {
    startIndex = 0;
    endIndex = 0;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void oneSegmentItin_inCorrectLocations1()
  {
    startIndex = 0;
    endIndex = 0;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void oneSegmentItin_inCorrectLocations2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentItin_correctLocations()
  {
    startIndex = 0;
    endIndex = 0;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentItin_stopInCorrectLocations()
  {
    startIndex = 1;
    endIndex = 1;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentItin_noStopInCorrectLocations()
  {
    startIndex = 1;
    endIndex = 1;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentItin_hiddenCity()
  {
    startIndex = 0;
    endIndex = 0;
    TaxXG_10m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 0;
    item.nextSubSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxXG_10Test);
};
