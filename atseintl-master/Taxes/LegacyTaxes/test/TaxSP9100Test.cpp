#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxSP9100.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DBAccess/Loc.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/test/TaxLocIteratorMock.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/LocRestrictionValidator.h"

namespace tse
{

class TaxSP9100m : public TaxSP9100
{
public:
  TaxLocIteratorMock _locItMock;
  bool _isUSCA;

protected:
  TaxLocIterator* getLocIterator(FarePath& farePath) { return &_locItMock; }

  bool isUSCA(const Loc& loc) { return _isUSCA; }

  bool isMirrorImage(PricingTrx& trx,
                     TaxResponse& taxResponse,
                     TaxCodeReg& taxCodeReg,
                     uint16_t travelSegIndex)
  {
    return false;
  }
};

class TaxSP9100Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP9100Test);
  CPPUNIT_TEST(stop);
  CPPUNIT_TEST(transit);
  CPPUNIT_TEST(international_itin);
  CPPUNIT_TEST(domestic_itin);
  CPPUNIT_TEST_SUITE_END();

  PricingTrx* trx;
  TaxResponse* taxResponse;
  TaxCodeReg* taxCodereg;
  uint16_t startIndex;
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

  void stop()
  {
    startIndex = 1;
    TaxSP9100m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.stop = true;
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.stop = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTransit(*trx, *taxResponse, *taxCodereg, startIndex);
    CPPUNIT_ASSERT(res);
  }

  void transit()
  {
    startIndex = 1;
    TaxSP9100m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.stop = true;
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.stop = false;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTransit(*trx, *taxResponse, *taxCodereg, startIndex);
    CPPUNIT_ASSERT(!res);
  }

  void international_itin()
  {
    startIndex = 1;
    TaxSP9100m tax;
    tax._isUSCA = false;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.stop = true;
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.stop = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    tax.validateTransit(*trx, *taxResponse, *taxCodereg, startIndex);
    CPPUNIT_ASSERT_EQUAL(24, int(tax._locItMock._stopHours));
  }

  void domestic_itin()
  {
    startIndex = 1;
    TaxSP9100m tax;
    tax._isUSCA = true;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.stop = true;
    item.loc = loc;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 1;
    item.stop = true;
    tax._locItMock.addLoc(item);

    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    tax.validateTransit(*trx, *taxResponse, *taxCodereg, startIndex);
    CPPUNIT_ASSERT_EQUAL(4, int(tax._locItMock._stopHours));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP9100Test);
}
