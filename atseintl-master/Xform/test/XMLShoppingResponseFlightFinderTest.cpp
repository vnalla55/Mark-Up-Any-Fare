#include "Common/Global.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareInfo.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Xform/XMLShoppingResponse.h"

using namespace std;
using namespace boost;

namespace tse
{

namespace
{

class MockXMLShoppingResponse : public XMLShoppingResponse
{
public:
  MockXMLShoppingResponse(tse::PricingTrx& trx) : XMLShoppingResponse(trx) {}
  ~MockXMLShoppingResponse() {}
  void testGenerateDFL() { generateDFL(); }
  void testGenerateOBG(CalcTotals& calc) { generateOBG(calc); }
  bool testShowFareBasisCode(FlightFinderTrx& fFTrx, bool outbound)
  {
    return showFareBasisCode(fFTrx, outbound);
  }
  bool testShowSOPs(FlightFinderTrx& fFTrx, bool outbound) { return showSOPs(fFTrx, outbound); }
  std::string testGetXML() { return _writer.result(); }
  uint32_t countFFSolutionNumber() { return XMLShoppingResponse::countFFSolutionNumber(); }
  uint32_t getOriginalSopIndex(FlightFinderTrx& flightFinderTrx,
                               const uint32_t legId,
                               const uint32_t sopIndex)
  {
    return XMLShoppingResponse::getOriginalSopIndex(flightFinderTrx, legId, sopIndex);
  }
};

} // namespace

class XMLShoppingResponseFlightFinderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLShoppingResponseFlightFinderTest);
  CPPUNIT_SKIP_TEST(testGenerateDFL);
  CPPUNIT_TEST(testGenerateOBG_TktFeeEmpty);
  CPPUNIT_TEST(testGenerateOBG_TktFeeOne);
  CPPUNIT_TEST(testGenerateDFL_FFMapEmpty);
  CPPUNIT_TEST(testGenerateDFL_FFLogicOFF);
  CPPUNIT_TEST(testNumberOfInboundFlights_FFMapEmpty);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_1_OutboundOnly);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_1_OutboundWithInbounds);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_2_OutboundOnly);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_2_OutboundWithInbounds);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_3_OutboundOnly);
  CPPUNIT_TEST(testNumberOfInboundFlights_STEP_3_OutboundWithInbounds);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_isFF);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_1);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_2);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_3);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_4);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_5);
  CPPUNIT_TEST(testShowFareBasisCode_Outbound_Step_6);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_isFF);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_1);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_2);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_3);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_4);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_5);
  CPPUNIT_TEST(testShowFareBasisCode_Inbound_Step_6);
  CPPUNIT_TEST(testShowSOPs_Outbound_isFF);
  CPPUNIT_TEST(testShowSOPs_Inbound_isFF);
  CPPUNIT_TEST(testGetOriginalSopIndex_outbound);
  CPPUNIT_TEST(testGetOriginalSopIndex_inbound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fFTrx = _memHandle.create<FlightFinderTrx>();
    _fFTrx->setTrxType(PricingTrx::FF_TRX);
    _paxTypeFare = buildFakePaxTypeFare();
    _fFTrx->legs().push_back(buildLegWithEmptySOPs());
    _fFTrx->legs().push_back(buildLegWithEmptySOPs());
    _fFTrx->setRequest(&_request);
  }

  void tearDown() { _memHandle.clear(); }

  void testNumberOfInboundFlights_FFMapEmpty()
  {
    // do not set data
    uint32_t expectedFFNumber = 0;
    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_1_OutboundOnly()
  {
    // 2007-09-20
    //   1 2
    DateTime outboundDT(2007, 9, 20);
    addOutboundToFlight(*_fFTrx, outboundDT, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT, 2, _paxTypeFare);
    uint32_t expectedFFNumber = 2;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_1_OutboundWithInbounds()
  {
    // 2007-09-20
    //   1 2
    //  2007-09-21
    //    1 2 3
    DateTime outboundDT(2007, 9, 20);
    addOutboundToFlight(*_fFTrx, outboundDT, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT, 2, _paxTypeFare);
    DateTime inboundDT(2007, 9, 21);
    addInboundToFlight(*_fFTrx, outboundDT, inboundDT, 1, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT, inboundDT, 2, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT, inboundDT, 3, _paxTypeFare);
    uint32_t expectedFFNumber = 6;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_2_OutboundOnly()
  {
    // 2007-09-20
    // 2007-10-20
    DateTime outboundDT1(2007, 9, 20);
    DateTime outboundDT2(2007, 10, 20);
    addOutboundToFlight(*_fFTrx, outboundDT1, 0, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, 0, _paxTypeFare);
    uint32_t expectedFFNumber = 2;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_2_OutboundWithInbounds()
  {
    // 2007-09-20
    //  2007-09-21
    //  2007-10-22
    // 2007-10-20
    //  2007-11-22
    DateTime outboundDT1(2007, 9, 20);
    DateTime outboundDT2(2007, 10, 20);
    addOutboundToFlight(*_fFTrx, outboundDT1, 0, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, 0, _paxTypeFare);
    DateTime inboundDT11(2007, 9, 21);
    DateTime inboundDT12(2007, 10, 22);
    DateTime inboundDT13(2007, 11, 22);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT11, 0, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT12, 0, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT2, inboundDT13, 0, _paxTypeFare);
    uint32_t expectedFFNumber = 3;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_3_OutboundOnly()
  {
    // 2007-09-20
    //   1 2
    // 2007-10-20
    //   1 2
    DateTime outboundDT1(2007, 9, 20);
    addOutboundToFlight(*_fFTrx, outboundDT1, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT1, 2, _paxTypeFare);
    DateTime outboundDT2(2007, 10, 20);
    addOutboundToFlight(*_fFTrx, outboundDT2, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, 2, _paxTypeFare);
    uint32_t expectedFFNumber = 4;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testNumberOfInboundFlights_STEP_3_OutboundWithInbounds()
  {
    // 2007-09-20
    //   1 2
    //  2007-09-21
    //    1 2
    //  2007-09-22
    //    3
    // 2007-10-20
    //   1 2
    //  2007-09-22
    DateTime outboundDT1(2007, 9, 20);
    addOutboundToFlight(*_fFTrx, outboundDT1, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT1, 2, _paxTypeFare);
    DateTime outboundDT2(2007, 10, 20);
    addOutboundToFlight(*_fFTrx, outboundDT2, 1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, 2, _paxTypeFare);
    DateTime inboundDT11(2007, 9, 21);
    DateTime inboundDT12(2007, 9, 22);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT11, 1, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT11, 2, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT12, 3, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT2, inboundDT12, 2, _paxTypeFare);
    uint32_t expectedFFNumber = 8;

    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t countedFFNumber = response.countFFSolutionNumber();

    CPPUNIT_ASSERT_EQUAL(expectedFFNumber, countedFFNumber);
  }

  void testGenerateDFL()
  {
    // Set data
    DateTime outboundDT1(2007, 9, 20, 14, 40, 0);
    DateTime inboundDT11(2007, 9, 21, 14, 40, 0);
    DateTime inboundDT12(2007, 9, 22, 14, 40, 0);
    DateTime outboundDT2(2007, 9, 22, 14, 40, 0);
    DateTime inboundDT2(2007, 9, 23, 14, 40, 0);
    uint32_t sop0 = 1;
    uint32_t sop1 = 2;

    addOutboundToFlight(*_fFTrx, outboundDT1, sop0, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT1, sop1, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT11, sop0, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT1, inboundDT12, sop1, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, sop0, _paxTypeFare);
    addOutboundToFlight(*_fFTrx, outboundDT2, sop1, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT2, inboundDT2, sop0, _paxTypeFare);
    addInboundToFlight(*_fFTrx, outboundDT2, inboundDT2, sop1, _paxTypeFare);

    MockXMLShoppingResponse response(*_fFTrx);
    response.testGenerateDFL();

    stringstream expected;
    expected << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    expected << "<DFL>" << endl;
    expected << "  <OBL D01=\"2007-09-20\"/>" << endl;
    expected << "  <FID Q14=\"0\" Q15=\"0\"/>" << endl;
    expected << "  <FID Q14=\"0\" Q15=\"1\"/>" << endl;
    expected << "  <IBL D01=\"2007-Sep-21 14:40:00\"/>" << endl;
    expected << "  <FID Q14=\"1\" Q15=\"0\"/>" << endl;
    expected << "  <FID Q14=\"1\" Q15=\"1\"/>" << endl;
    expected << "</DFL>" << endl;

    CPPUNIT_ASSERT(response.testGetXML() != "");
    // CPPUNIT_ASSERT_EQUAL(response.testGetXML(), expected.str());
  }

  void testGenerateDFL_FFLogicOFF()
  {
    _fFTrx->setTrxType(PricingTrx::PRICING_TRX);

    MockXMLShoppingResponse response(*_fFTrx);
    response.testGenerateDFL();

    stringstream expected;
    expected << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    CPPUNIT_ASSERT(response.testGetXML() != "");
    CPPUNIT_ASSERT_EQUAL(response.testGetXML(), expected.str());
  }

  void testGenerateOBG_TktFeeEmpty()
  {
    MockXMLShoppingResponse response(*buildShoppingTrxForOBG());
    response.testGenerateOBG(*buildCalcTotalWithNoElement());

    stringstream expected;
    expected << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    CPPUNIT_ASSERT(response.testGetXML() != "");
    CPPUNIT_ASSERT_EQUAL(expected.str(), response.testGetXML());
  }

  void testGenerateOBG_TktFeeOne()
  {
    MockXMLShoppingResponse response(*buildShoppingTrxForOBG());
    response.testGenerateOBG(*buildCalcTotalWithOneElement());

    stringstream expected;
    expected << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><OBG>" << endl;
    expected << "  <OBF SF1=\"10.000000\"/>" << endl;
    expected << "</OBG>" << endl;

    CPPUNIT_ASSERT(response.testGetXML() != "");
    CPPUNIT_ASSERT_EQUAL(expected.str(), response.testGetXML());
  }

  void testGenerateDFL_FFMapEmpty()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    response.testGenerateDFL();

    stringstream expected;
    expected << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

    CPPUNIT_ASSERT(response.testGetXML() != "");
    CPPUNIT_ASSERT_EQUAL(response.testGetXML(), expected.str());
  }

  void testShowFareBasisCode_Outbound_isFF()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_1()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_1;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_1);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_2()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_2;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_2);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_3()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_3;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_3);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_4()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_4;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_4);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_5()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_5;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_5);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Outbound_Step_6()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_6;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_6);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, true));
  }

  void testShowFareBasisCode_Inbound_isFF()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_1()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_1;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_1);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_2()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_2;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_2);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_3()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_3;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_3);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_4()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_4;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_4);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_5()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_5;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_5);
    CPPUNIT_ASSERT(!response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowFareBasisCode_Inbound_Step_6()
  {
    _fFTrx->bffStep() = FlightFinderTrx::STEP_6;
    _fFTrx->owrt() = "O";

    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT_EQUAL(_fFTrx->bffStep(), FlightFinderTrx::STEP_6);
    CPPUNIT_ASSERT(response.testShowFareBasisCode(*_fFTrx, false));
  }

  void testShowSOPs_Outbound_isFF()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT(response.testShowSOPs(*_fFTrx, true));
  }

  void testShowSOPs_Inbound_isFF()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    CPPUNIT_ASSERT(response.testShowSOPs(*_fFTrx, false));
  }

  void testGetOriginalSopIndex_outbound()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t expectedOriginalSOPIndex = 1;

    CPPUNIT_ASSERT_EQUAL(response.getOriginalSopIndex(*_fFTrx, 0, 1), expectedOriginalSOPIndex);
  }

  void testGetOriginalSopIndex_inbound()
  {
    MockXMLShoppingResponse response(*_fFTrx);
    uint32_t expectedOriginalSOPIndex = 1;

    CPPUNIT_ASSERT_EQUAL(response.getOriginalSopIndex(*_fFTrx, 1, 1), expectedOriginalSOPIndex);
  }

protected:
  ShoppingTrx::Leg& buildLegWithEmptySOPs()
  {
    ShoppingTrx::Leg* leg = 0;
    _fFTrx->dataHandle().get(leg);
    ShoppingTrx::SchedulingOption sop(0, 1, true);
    leg->sop().push_back(sop);
    leg->sop().push_back(sop);
    leg->sop().push_back(sop);

    return *leg;
  }

  void addOutboundToFlight(FlightFinderTrx& fFTrx,
                           const DateTime& outboundDepartureDT,
                           const uint32_t originalSopIndex,
                           PaxTypeFare* paxTypeFare)
  {
    if (fFTrx.outboundDateflightMap().count(outboundDepartureDT) == 0)
    {
      FlightFinderTrx::OutBoundDateInfo* outBoundDateInfoPtr = 0;
      fFTrx.dataHandle().get(outBoundDateInfoPtr);
      FlightFinderTrx::SopInfo* sopInfo;
      fFTrx.dataHandle().get(sopInfo);

      sopInfo->sopIndex = originalSopIndex;
      sopInfo->paxTypeFareVect.push_back(paxTypeFare);
      std::vector<FlightFinderTrx::BookingCodeData> bkgDataVect;
      FlightFinderTrx::BookingCodeData bkgData;
      bkgData.bkgCode = BookingCode("AA");
      bkgData.numSeats = 7;
      bkgDataVect.push_back(bkgData);
      sopInfo->bkgCodeDataVect.push_back(bkgDataVect);
      outBoundDateInfoPtr->flightInfo.flightList.push_back(sopInfo);
      fFTrx.outboundDateflightMap()[outboundDepartureDT] = outBoundDateInfoPtr;
    }
    else
    {
      // if exist add next sopIndex to this date
      if (fFTrx.outboundDateflightMap()[outboundDepartureDT] != 0)
      {
        FlightFinderTrx::SopInfo* sopInfo;
        fFTrx.dataHandle().get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        std::vector<FlightFinderTrx::BookingCodeData> bkgDataVect;
        FlightFinderTrx::BookingCodeData bkgData;
        bkgData.bkgCode = BookingCode("AA");
        bkgData.numSeats = 7;
        bkgDataVect.push_back(bkgData);
        sopInfo->bkgCodeDataVect.push_back(bkgDataVect);
        fFTrx.outboundDateflightMap()[outboundDepartureDT]->flightInfo.flightList.push_back(
            sopInfo);
      }
    }
  }

  void addInboundToFlight(FlightFinderTrx& fFTrx,
                          const DateTime& outboundDepartureDT,
                          const DateTime& inboundDepartureDT,
                          const uint32_t originalSopIndex,
                          PaxTypeFare* paxTypeFare)
  {
    FlightFinderTrx::OutBoundDateFlightMap::const_iterator it =
        fFTrx.outboundDateflightMap().find(outboundDepartureDT);
    if (it != fFTrx.outboundDateflightMap().end())
    {
      if (it->second->iBDateFlightMap.count(inboundDepartureDT) == 0)
      {
        FlightFinderTrx::FlightDataInfo* flightInfo = 0;
        fFTrx.dataHandle().get(flightInfo);
        FlightFinderTrx::SopInfo* sopInfo;
        fFTrx.dataHandle().get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        std::vector<FlightFinderTrx::BookingCodeData> bkgDataVect;
        FlightFinderTrx::BookingCodeData bkgData;
        bkgData.bkgCode = BookingCode("AA");
        bkgData.numSeats = 7;
        bkgDataVect.push_back(bkgData);
        sopInfo->bkgCodeDataVect.push_back(bkgDataVect);
        flightInfo->flightList.push_back(sopInfo);
        it->second->iBDateFlightMap[inboundDepartureDT] = flightInfo;
      }
      else
      {
        // add to existing one
        FlightFinderTrx::SopInfo* sopInfo;
        fFTrx.dataHandle().get(sopInfo);

        sopInfo->sopIndex = originalSopIndex;
        sopInfo->paxTypeFareVect.push_back(paxTypeFare);
        std::vector<FlightFinderTrx::BookingCodeData> bkgDataVect;
        FlightFinderTrx::BookingCodeData bkgData;
        bkgData.bkgCode = BookingCode("AA");
        bkgData.numSeats = 7;
        bkgDataVect.push_back(bkgData);
        sopInfo->bkgCodeDataVect.push_back(bkgDataVect);
        it->second->iBDateFlightMap[inboundDepartureDT]->flightList.push_back(sopInfo);
      }
    }
  }

  PaxTypeFare* buildFakePaxTypeFare()
  {
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    FareMarket* fM = _memHandle.create<FareMarket>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->fareClass() = "TESTFARECLASS";
    fare->initialize(Fare::FS_ConstructedFare, fareInfo, *fM, 0, 0);
    paxTypeFare->initialize(fare, 0, fM);

    return paxTypeFare;
  }

  PricingTrx* buildShoppingTrxForOBG()
  {
    ShoppingTrx* trx = _memHandle.create<ShoppingTrx>();
    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->collectOBFee() = 'T';
    request->validatingCarrier() = "AA";
    trx->setRequest(request);

    return trx;
  }

  CalcTotals* buildCalcTotalWithNoElement()
  {
    CalcTotals* calc = _memHandle.create<CalcTotals>();
    FarePath* fp = _memHandle.create<FarePath>();
    calc->farePath = fp;

    return calc;
  }
  CalcTotals* buildCalcTotalWithOneElement()
  {
    CalcTotals* calc = _memHandle.create<CalcTotals>();
    FarePath* fp = _memHandle.create<FarePath>();
    calc->farePath = fp;
    TicketingFeesInfo* feeInfo = _memHandle.create<TicketingFeesInfo>();
    feeInfo->feeAmount() = 10;
    fp->collectedTktOBFees().push_back(feeInfo);
    return calc;
  }

private:
  FlightFinderTrx* _fFTrx;
  std::vector<size_t> _allocData;
  PaxTypeFare* _paxTypeFare;
  TestMemHandle _memHandle;
  PricingRequest _request;
};

CPPUNIT_TEST_SUITE_REGISTRATION(XMLShoppingResponseFlightFinderTest);

} // tse
