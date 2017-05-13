#include "test/include/CppUnitHelperMacros.h"

#include <vector>
#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/Itin.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/MockTseServer.h"
#include "ItinAnalyzer/FareDisplayAnalysis.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "DataModel/FareMarket.h"
#include <iostream>
#include "test/include/TestMemHandle.h"

namespace tse
{

class FareDisplayAnalysisTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(FareDisplayAnalysisTest);

  CPPUNIT_TEST(testCTRW);
  CPPUNIT_TEST(testDefault_NLX);
  CPPUNIT_TEST(testDefault_ALL);
  CPPUNIT_TEST(testMatchAppend_ChNoPercent);
  CPPUNIT_TEST(testMatchAppend_Ch2Num);
  CPPUNIT_TEST(testMatchAppend_InNoPercent);
  CPPUNIT_TEST(testMatchAppend_In1Num);
  CPPUNIT_TEST(testMatchAppend_In2Num);
  CPPUNIT_TEST(testMatchAppend_NoMatch1);
  CPPUNIT_TEST(testMatchAppend_NoMatch2);
  CPPUNIT_TEST(testMatchAppend_Empty);
  CPPUNIT_TEST(testMatchAppend_Ch3Num);
  CPPUNIT_TEST(testWebInclMulticity1);
  CPPUNIT_TEST(testWebInclMulticity2);
  CPPUNIT_TEST(testWebInclMulticity3);
  CPPUNIT_TEST(testWebInclMulticity4);
  CPPUNIT_TEST(testWebInclPsgType);

  CPPUNIT_TEST_SUITE_END();

public:
  // used for all tests
  void setUp()
  {
    _trx = _memHandle.create<FareDisplayTrx>();
    _request = _memHandle.create<FareDisplayRequest>();
    _options = _memHandle.create<FareDisplayOptions>();
    _trx->setOptions(_options);
    _trx->setRequest(_request);
    _analysis = _memHandle.insert(new FareDisplayAnalysis(*_trx));
  }
  void tearDown() { _memHandle.clear(); }
  void testCTRW()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    FareDisplayOptions options;
    AirSeg tvlSeg;
    Loc loc1;
    Loc loc2;
    loc1.loc() = "ABC";
    loc2.loc() = "ABC";

    tvlSeg.origin() = &loc1;
    tvlSeg.destination() = &loc2;
    trx.travelSeg().push_back(&tvlSeg);

    trx.setRequest(&request);
    trx.setOptions(&options);

    FareDisplayAnalysis analysis(trx);
    CPPUNIT_ASSERT(request.inclusionCode().empty());
    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "NLX");
  }
  void testDefault_NLX()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    FareDisplayOptions options;
    AirSeg tvlSeg;
    Loc loc1;
    Loc loc2;
    loc1.loc() = "ABC";
    loc2.loc() = "DEF";

    tvlSeg.boardMultiCity() = "ABC";
    tvlSeg.offMultiCity() = "DEF";
    tvlSeg.origin() = &loc1;
    tvlSeg.destination() = &loc2;
    trx.travelSeg().push_back(&tvlSeg);

    trx.setRequest(&request);
    trx.setOptions(&options);

    FareDisplayAnalysis analysis(trx);
    CPPUNIT_ASSERT(request.inclusionCode().empty());
    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "NLX");
  }
  void testDefault_ALL()
  {
    FareDisplayTrx trx;
    FareDisplayRequest request;
    FareDisplayOptions options;
    AirSeg tvlSeg;
    Loc loc1;
    Loc loc2;
    loc1.loc() = "ABC";
    loc2.loc() = "DEF";

    tvlSeg.boardMultiCity() = "ABC";
    tvlSeg.offMultiCity() = "DEF";
    tvlSeg.origin() = &loc1;
    tvlSeg.destination() = &loc2;
    trx.travelSeg().push_back(&tvlSeg);

    trx.setRequest(&request);
    trx.setOptions(&options);

    FareDisplayAnalysis analysis(trx);
    CPPUNIT_ASSERT(request.inclusionCode().empty());

    request.fareBasisCode() = "AB";
    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "ALL");

    // case-2
    request.fareBasisCode().clear();
    request.inclusionCode().clear();

    request.ticketDesignator() = "AB";

    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "ALL");

    // case-3
    request.inclusionCode().clear();
    request.ticketDesignator().clear();

    request.bookingCode() == "b";
    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "NLX");

    // case-4
    request.inclusionCode().clear();
    request.bookingCode().clear();
    request.displayPassengerTypes().push_back("ADT");
    _analysis->setInclusionCode(request, options);
    CPPUNIT_ASSERT(!request.inclusionCode().empty());
    CPPUNIT_ASSERT(request.inclusionCode() == "ALL");
  }

  // pass cases with CH
  void testMatchAppend_ChNoPercent()
  {
    std::string s = std::string("YCH");
    CPPUNIT_ASSERT(_analysis->matchChInAppend(s, true));
  }
  void testMatchAppend_Ch2Num()
  {
    std::string s = std::string("BLAMCH75");
    CPPUNIT_ASSERT(_analysis->matchChInAppend(s, true));
  }
  // pass cases with IN
  void testMatchAppend_InNoPercent()
  {
    std::string s = std::string("WOOIN");
    CPPUNIT_ASSERT(_analysis->matchChInAppend(s, false));
  }
  void testMatchAppend_In1Num()
  {
    std::string s = std::string("YABADABADOOIN0");
    CPPUNIT_ASSERT(_analysis->matchChInAppend(s, false));
  }
  void testMatchAppend_In2Num()
  {
    std::string s = std::string("BLAMIN75");
    CPPUNIT_ASSERT(_analysis->matchChInAppend(s, false));
  }
  // fails cases
  void testMatchAppend_NoMatch1()
  {
    std::string s = std::string("CHINCHINREVENGE");
    CPPUNIT_ASSERT(!_analysis->matchChInAppend(s, true));
  }
  void testMatchAppend_NoMatch2()
  {
    std::string s = std::string("Y");
    CPPUNIT_ASSERT(!_analysis->matchChInAppend(s, true));
  }
  void testMatchAppend_Empty()
  {
    std::string s;
    CPPUNIT_ASSERT(!_analysis->matchChInAppend(s, false));
  }
  void testMatchAppend_Ch3Num()
  {
    std::string s = std::string("3DIGITSCH123");
    CPPUNIT_ASSERT(!_analysis->matchChInAppend(s, true));
  }
  void testWebInclMulticity1()
  {
    Loc loc1;
    Loc loc2;
    loc1.loc() = "DFW";
    loc2.loc() = "TYO";
    AirSeg airSeg;
    airSeg.origin() = &loc1;
    airSeg.destination() = &loc2;
    _trx->travelSeg().clear();
    _trx->travelSeg().push_back(&airSeg);
    DateTime time1(2008, 11, 11);
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->getRequest()->requestedDepartureDT() = time1;
    _trx->getOptions()->allCarriers() = 'Y';
    CPPUNIT_ASSERT_THROW(_analysis->checkWebInclCd(), ErrorResponseException);
  }

  void testWebInclMulticity2()
  {
    Loc loc1;
    Loc loc2;
    loc1.loc() = "DFW";
    loc2.loc() = "TYO";
    AirSeg airSeg;
    airSeg.origin() = &loc1;
    airSeg.destination() = &loc2;
    _trx->travelSeg().empty();
    _trx->travelSeg().push_back(&airSeg);
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->getOptions()->allCarriers() = 'N';
    _trx->preferredCarriers().clear();
    _trx->preferredCarriers().insert("LO");
    _trx->preferredCarriers().insert("AA");
    _trx->preferredCarriers().insert("UA");
    CPPUNIT_ASSERT_THROW(_analysis->checkWebInclCd(), ErrorResponseException);
  }

  void testWebInclMulticity3()
  {
    Loc loc1;
    Loc loc2;
    loc1.loc() = "DFW";
    loc2.loc() = "TYO";
    AirSeg airSeg;
    airSeg.origin() = &loc1;
    airSeg.destination() = &loc2;
    _trx->travelSeg().empty();
    _trx->travelSeg().push_back(&airSeg);
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->getOptions()->allCarriers() = 'N';
    _trx->preferredCarriers().clear();
    _trx->preferredCarriers().insert("**");

    CPPUNIT_ASSERT_THROW(_analysis->checkWebInclCd(), ErrorResponseException);
  }

  void testWebInclMulticity4()
  {
    Loc loc1;
    Loc loc2;
    loc1.loc() = "DFW";
    loc2.loc() = "ORD";
    AirSeg airSeg;
    airSeg.origin() = &loc1;
    airSeg.destination() = &loc2;
    _trx->travelSeg().empty();
    _trx->travelSeg().push_back(&airSeg);
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->getOptions()->allCarriers() = 'N';
    _trx->preferredCarriers().clear();
    _trx->preferredCarriers().insert("AA");

    CPPUNIT_ASSERT_NO_THROW(_analysis->checkWebInclCd());
  }

  void testWebInclPsgType()
  {
    _trx->getRequest()->inclusionCode() = "WEB";
    _trx->getOptions()->allCarriers() = 'N';
    _trx->preferredCarriers().clear();
    _trx->preferredCarriers().insert("AA");
    _request->displayPassengerTypes().push_back("ADT");

    CPPUNIT_ASSERT_THROW(_analysis->checkWebInclCd(), ErrorResponseException);
  }

private:
  FareDisplayTrx* _trx;
  FareDisplayAnalysis* _analysis;
  FareDisplayRequest* _request;
  FareDisplayOptions* _options;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayAnalysisTest);
}
