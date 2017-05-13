#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxSP9000.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/test/TaxLocIteratorMock.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"

namespace tse
{

class TaxSP9000m : public TaxSP9000
{
public:
  TaxLocIteratorMock _locItMock;

protected:
  TaxLocIterator* getLocIterator(FarePath& farePath) { return &_locItMock; }
};

class TaxSP9000Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP9000Test);
  CPPUNIT_TEST(validateTripTypes_oneSegmentItin_correctLocations);
  CPPUNIT_TEST(validateTripTypes_oneSegmentItin_inCorrectLocations1);
  CPPUNIT_TEST(validateTripTypes_oneSegmentItin_inCorrectLocations2);
  CPPUNIT_TEST(validateTripTypes_twoSegmentItin_correctLocations);
  CPPUNIT_TEST(validateTripTypes_twoSegmentItin_noNextStop);
  CPPUNIT_TEST(validateTripTypes_twoSegmentItin_nextStop);
  CPPUNIT_TEST(validateTripTypes_twoSegmentItin_hiddenCity);
  CPPUNIT_TEST(validateTripTypes_twoSegmentItin_hiddenCity2);
  CPPUNIT_TEST(validateTransitOnHiddenPoints_enplanement_hiddenBrd);
  CPPUNIT_TEST(validateTransitOnHiddenPoints_enplanement_hiddenOff);
  CPPUNIT_TEST(validateTransitOnHiddenPoints_deplanement_hiddenBrd);
  CPPUNIT_TEST(validateTransitOnHiddenPoints_deplanement_hiddenOff);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx* trx;
  TaxResponse* taxResponse;
  TaxCodeReg* taxCodereg;
  uint16_t startIndex, endIndex;
  Loc* loc;

public:
  void setUp()
  {
    trx = new PricingTrx();
    taxResponse = new TaxResponse();
    taxCodereg = new TaxCodeReg();
    loc = new Loc();
    loc->loc() = "KTW";
  }

  void tearDown()
  {
    delete taxCodereg;
    delete taxResponse;
    delete trx;
    delete loc;
  }

  void validateTripTypes_oneSegmentItin_correctLocations()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

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
    CPPUNIT_ASSERT(res);
  }

  void validateTripTypes_oneSegmentItin_inCorrectLocations1()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

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

  void validateTripTypes_oneSegmentItin_inCorrectLocations2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

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

  void validateTripTypes_twoSegmentItin_correctLocations()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
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
    CPPUNIT_ASSERT(res);
  }

  void validateTripTypes_twoSegmentItin_noNextStop()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;
    taxCodereg->nextstopoverrestr() = NO;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = false;
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

  void validateTripTypes_twoSegmentItin_nextStop()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;
    taxCodereg->nextstopoverrestr() = YES;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
  }

  void validateTripTypes_twoSegmentItin_hiddenCity()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 0;
    item.nextSubSegNo = 1;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = false;
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = true;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
  }

  void validateTripTypes_twoSegmentItin_hiddenCity2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9000m tax;

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
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.nextSubSegNo = 0;
    item.isLoc1 = false;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    // tax._locItMock.printLocs();

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodereg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
  }

  void validateTransitOnHiddenPoints_enplanement_hiddenBrd()
  {
    TaxSP9000 tax;
    taxCodereg->loc1Appl() = LocRestrictionValidator::TAX_ENPLANEMENT;
    tax._hiddenBrdAirport = "KTW";
    bool res = tax.validateTransitOnHiddenPoints(*taxCodereg);
    CPPUNIT_ASSERT(res);
  }

  void validateTransitOnHiddenPoints_enplanement_hiddenOff()
  {
    TaxSP9000 tax;
    taxCodereg->loc1Appl() = LocRestrictionValidator::TAX_ENPLANEMENT;
    tax._hiddenOffAirport = "KTW";
    bool res = tax.validateTransitOnHiddenPoints(*taxCodereg);
    CPPUNIT_ASSERT(!res);
  }

  void validateTransitOnHiddenPoints_deplanement_hiddenBrd()
  {
    TaxSP9000 tax;
    taxCodereg->loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;
    tax._hiddenBrdAirport = "KTW";
    bool res = tax.validateTransitOnHiddenPoints(*taxCodereg);
    CPPUNIT_ASSERT(!res);
  }

  void validateTransitOnHiddenPoints_deplanement_hiddenOff()
  {
    TaxSP9000 tax;
    taxCodereg->loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;
    tax._hiddenOffAirport = "KTW";
    bool res = tax.validateTransitOnHiddenPoints(*taxCodereg);
    CPPUNIT_ASSERT(res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP9000Test);
};
