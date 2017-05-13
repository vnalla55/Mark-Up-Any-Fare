#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxSP9300.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/test/TaxLocIteratorMock.h"
#include "DBAccess/TaxCodeReg.h"
#include "test/testdata/TestLocFactory.h"

namespace tse
{

class TaxSP9300m : public TaxSP9300
{
public:
  TaxLocIteratorMock _locItMock;

protected:
  TaxLocIterator* getLocIterator(FarePath& farePath) { return &_locItMock; }

  bool isStop(PricingTrx& trx, TaxLocIterator& locIt, TaxCodeReg& taxCodeReg)
  {
    return locIt.isStop();
  }
};

class TaxLocIteratorMockSP9300 : public TaxLocIteratorMock
{
public:
  virtual bool isStop()
  {
    if (_stopHours <= 12)
      return true;
    return false;
  }
};

class TaxSP9300Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP9300Test);
  CPPUNIT_TEST(oneSegmentItin_DE_to_DE_seqLoc2DE);
  CPPUNIT_TEST(oneSegmentItin_DE_to_NotDE_seqLoc2DE);
  CPPUNIT_TEST(twoSegmentsItin_DE_to_DE_to_DE_seqLoc2DE);
  CPPUNIT_TEST(twoSegmentsItin_DE_to_DE_to_NotDE_seqLoc2DE);
  CPPUNIT_TEST(oneSegmentItin_DE_to_DE_seqLoc2NotDE);
  CPPUNIT_TEST(oneSegmentItin_DE_to_NotDE_loc2Match);
  CPPUNIT_TEST(oneSegmentItin_DE_to_NotDE_loc2NotMatch);
  CPPUNIT_TEST(twoSegmentItin_DE_to_NotDE_NoDEBeforeMatch);
  CPPUNIT_TEST(twoSegmentItin_DE_to_NotDE_DEConxBeforeMatch);
  CPPUNIT_TEST(twoSegmentItin_DE_to_NotDE_DEStopBeforeMatch);
  CPPUNIT_TEST(threeSegmentItin_DE_to_NotDE_DEStop_Loc2);
  CPPUNIT_TEST(threeSegmentItin_DE_to_NotDE_DEConx_Loc2);
  CPPUNIT_TEST(threeSegmentItin_DE_to_NotDE_DEConxWithARNK_Loc2);
  CPPUNIT_TEST(testTransitValidationSuccess);
  CPPUNIT_TEST(testTransitValidationFailure);
  CPPUNIT_TEST(testStopoverNoPrevious);
  CPPUNIT_TEST(testStopoverNoNext);
  CPPUNIT_TEST(testStopoverMiddleNoRestrictions);
  CPPUNIT_TEST(testStopoverMiddle12RestrictionWithoutViaLoc);
  CPPUNIT_TEST(testStopoverMiddle24RestrictionWithoutViaLoc);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocDE);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocFR);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocUS);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocUSNotAir);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocUSMirror);
  CPPUNIT_TEST(testStopoverMiddleRestrictionViaLocUSTrain);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* trx;
  TaxResponse* taxResponse;
  TaxCodeReg* taxCodeReg;
  uint16_t startIndex, endIndex;
  Loc* locDE;
  Loc* locFR;
  Loc* locUS;
  AirSeg* seg;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    trx = _memHandle.create<PricingTrx>();
    PricingRequest* pr = _memHandle.create<PricingRequest>();
    taxResponse = _memHandle.create<TaxResponse>();
    taxCodeReg = _memHandle.create<TaxCodeReg>();
    trx->setRequest(pr);

    locDE = TestLocFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/0_FRA_CDG_DFW_ORD_0_Loc_FRA.xml");
    locFR = TestLocFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/0_FRA_CDG_DFW_ORD_0_Loc_CDG.xml");
    locUS = TestLocFactory::create(
        "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/0_FRA_CDG_DFW_ORD_1_Loc_DFW.xml");

    addTransitRestriction("DE", NATION, 12);
    addTransitRestriction("FR", NATION, 12);
    addTransitRestriction("US", NATION, 24);

    seg = _memHandle.create<AirSeg>();
  }

  void tearDown() { _memHandle.clear(); }

  void addTransitRestriction(const std::string& viaLoc, LocType viaLocType, int transitHours = 24)
  {
    TaxRestrictionTransit tr;
    tr.viaLoc() = viaLoc;
    tr.viaLocType() = viaLocType;
    tr.transitHours() = transitHours;
    taxCodeReg->restrictionTransit().push_back(tr);
  }

  void clearTransitRestrictions() { taxCodeReg->restrictionTransit().clear(); }

  void oneSegmentItin_DE_to_DE_seqLoc2DE()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "DE";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.stop = false;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
  }

  void oneSegmentItin_DE_to_NotDE_seqLoc2DE()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "DE";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc2 = false;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentsItin_DE_to_DE_to_DE_seqLoc2DE()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "DE";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.loc = locDE;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.stop = false;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(1 == endIndex);
  }

  void twoSegmentsItin_DE_to_DE_to_NotDE_seqLoc2DE()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "DE";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.loc = locDE;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.stop = false;
    tax._locItMock.addLoc(item);
    item.isLoc2 = false;
    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }
  // TaxCodeReg loc 2 not DE

  void oneSegmentItin_DE_to_DE_seqLoc2NotDE()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.loc = locDE;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void oneSegmentItin_DE_to_NotDE_loc2Match()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.loc = locFR;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(0 == endIndex);
  }

  void oneSegmentItin_DE_to_NotDE_loc2NotMatch()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.loc = locFR;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void twoSegmentItin_DE_to_NotDE_NoDEBeforeMatch()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = false;
    item.loc = locFR;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = false;
    item.loc = locFR;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(1 == endIndex);
  }

  void twoSegmentItin_DE_to_NotDE_DEConxBeforeMatch()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = false;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = false;
    item.loc = locFR;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(1 == endIndex);
  }

  void twoSegmentItin_DE_to_NotDE_DEStopBeforeMatch()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = false;
    item.loc = locFR;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void threeSegmentItin_DE_to_NotDE_DEStop_Loc2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locFR;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 3;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = true;
    item.loc = locUS;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void threeSegmentItin_DE_to_NotDE_DEConx_Loc2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locFR;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = false;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 3;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = true;
    item.loc = locUS;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(2 == endIndex);
  }

  void threeSegmentItin_DE_to_NotDE_DEConxWithARNK_Loc2()
  {
    startIndex = 0;
    endIndex = 0;
    TaxSP9300m tax;

    taxCodeReg->loc2() = "3";

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = true;
    item.loc = locFR;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.isLoc1 = false;
    item.isLoc2 = false;
    item.stop = false;
    item.airSeg = false;
    item.loc = locDE;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 3;
    item.isLoc1 = false;
    item.isLoc2 = true;
    item.stop = true;
    item.loc = locUS;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTripTypes(*trx, *taxResponse, *taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(!res);
  }

  void testTransitValidationSuccess()
  {
    startIndex = 0;
    TaxSP9300m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTransit(*trx, *taxResponse, *taxCodeReg, startIndex);
    CPPUNIT_ASSERT(res);
  }

  void testTransitValidationFailure()
  {
    startIndex = 1;
    TaxSP9300m tax;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    item.isLoc1 = true;
    item.isLoc2 = true;
    item.stop = true;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 1;
    item.stop = false;
    tax._locItMock.addLoc(item);
    item.nextSegNo = 2;
    item.stop = true;
    tax._locItMock.addLoc(item);

    bool res = tax.validateTransit(*trx, *taxResponse, *taxCodeReg, startIndex);
    CPPUNIT_ASSERT(!res);
  }

  void testStopoverNoPrevious()
  {
    TaxSP9300 tax;
    TaxLocIteratorMock locIt;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.nextSegNo = 1;
    locIt.addLoc(item);

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverNoNext()
  {
    TaxSP9300 tax;
    TaxLocIteratorMock locIt;

    TaxLocItMockItem item;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.nextSegNo = 1;
    locIt.addLoc(item);

    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleNoRestrictions()
  {
    clearTransitRestrictions();
    TaxSP9300 tax;
    TaxLocIteratorMock locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddle12RestrictionWithoutViaLoc()
  {
    clearTransitRestrictions();
    addTransitRestriction("", NATION, 12);

    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddle24RestrictionWithoutViaLoc()
  {
    clearTransitRestrictions();
    addTransitRestriction("", NATION, 24);

    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocDE()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.loc = locDE;
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.loc = locDE;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocFR()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.loc = locDE;
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.loc = locFR;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocUS()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.loc = locDE;
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.loc = locUS;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(!tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocUSNotAir()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.airSeg = false;
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.airSeg = true;
    item.loc = locDE;
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.loc = locUS;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocUSMirror()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    TaxLocItMockItem item;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 1;
    item.isMirror = true;
    locIt.addLoc(item);
    item.loc = locUS;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }

  void testStopoverMiddleRestrictionViaLocUSTrain()
  {
    TaxSP9300 tax;
    TaxLocIteratorMockSP9300 locIt;

    seg->equipmentType() = TRAIN;

    TaxLocItMockItem item;
    item.travelSeg = seg;
    item.loc = locDE;
    item.nextSegNo = 0;
    item.nextSubSegNo = 0;
    locIt.addLoc(item);
    item.loc = locDE;
    item.nextSegNo = 1;
    locIt.addLoc(item);
    item.loc = locUS;
    item.nextSegNo = 2;
    locIt.addLoc(item);

    locIt.toSegmentNo(0);
    locIt.next();

    CPPUNIT_ASSERT(tax.isStop(*trx, locIt, *taxCodeReg));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP9300Test);
};
