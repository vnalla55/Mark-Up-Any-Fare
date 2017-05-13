#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "Xform/PricingDetailResponseFormatter.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/SegmentDetail.h"
#include "DataModel/PaxDetail.h"
#include "test/include/TestMemHandle.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalcConsts.h"

using namespace std;

namespace tse
{
class MockPricingDetailResponseFormatter : public PricingDetailResponseFormatter
{
protected:
  CurrencyNoDec noCurrencyDec(const tse::CurrencyCode& code, const DateTime& ticketingDate) const
  {
    return 2;
  }
};

class PricingDetailResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingDetailResponseFormatterTest);
  CPPUNIT_TEST(testAddComponentPrefixInfo_SegmentDetailNotIn_MarketPosInFCLine);
  CPPUNIT_TEST(testAddComponentPrefixInfo_E);
  CPPUNIT_TEST(testAddComponentPrefixInfo_T);
  CPPUNIT_TEST(testAddComponentPrefixInfo_Stopover);
  CPPUNIT_TEST(testAddComponentPrefixInfo_Transfer);
  CPPUNIT_TEST(testCreateFCMarketMap_FareCalcDetailsEmpty);
  CPPUNIT_TEST(testCreateFCMarketMap_SegmentDetailsEmpty);
  CPPUNIT_TEST(testCreateFCMarketMap);
  CPPUNIT_TEST(testCreateFCMarketMap_2FareCalcDetails);

  CPPUNIT_TEST(testAddFareBreakPointInfo_Without_SideTrip);
  CPPUNIT_TEST(testAddFareBreakPointInfo_With_SideTrip);
  CPPUNIT_TEST(testAddFareBreakPointInfo_RW_SFC);
  CPPUNIT_TEST(testAddFareBreakPointInfo_CT_SFC);
  CPPUNIT_TEST(testAddPUType_RW_SFC);
  CPPUNIT_TEST(testAddPUType_CT_SFC);
  CPPUNIT_TEST_SUITE_END();

  // data
private:
  TestMemHandle _memHandle;
  PricingDetailResponseFormatter* _formatter;
  PricingDetailTrx* _pricingDetailTrx;
  SegmentDetail* _segmentDetail;
  PaxDetail* _paxDetail;
  FareCalcDetail* _fareCalcDetail;
  FareCalcConfig* _fcConfig;

  // helper methods
public:
  void setUp()
  {
    _formatter = _memHandle.insert(new MockPricingDetailResponseFormatter);
    _pricingDetailTrx = _memHandle.insert(new PricingDetailTrx);
    _segmentDetail = _memHandle.insert(new SegmentDetail);
    _paxDetail = _memHandle.insert(new PaxDetail);
    _fareCalcDetail = _memHandle.insert(new FareCalcDetail);
    _fcConfig = _memHandle.insert(new FareCalcConfig);
    _fcConfig->globalSidetripInd() = FareCalcConsts::FC_TWO;
  }

  void tearDown() { _memHandle.clear(); }

  void setFareCalcDetail()
  {
    _fareCalcDetail->fareBasisCode() = "REUNBA";
    _fareCalcDetail->fareComponentCurrencyCode() = "GBP";
    _fareCalcDetail->fareAmount() = 0.25;
    _fareCalcDetail->directionality() = "TO";
    _fareCalcDetail->arrivalCity() = "CPH";
    _fareCalcDetail->departureCity() = "LON";
    _fareCalcDetail->trueGoverningCarrier() = "BA";
    _fareCalcDetail->oneWayFare() = 'T';
    _fareCalcDetail->isRouting() = false;
    _fareCalcDetail->globalIndicator() = "EH";
  }

  void setSegmentDetail(const char stopover, const char transfer)
  {
    _segmentDetail->stopover() = stopover;
    _segmentDetail->transfer() = transfer;
    _segmentDetail->cityStopoverCharge() = 0.002;
    _segmentDetail->transferCharge() = 0.002;
    _segmentDetail->stopoverPubCurrency() = "USD";
    _segmentDetail->transferPubCurrency() = "USD";
    _segmentDetail->arrivalCity() = "NYC";
  }

  SegmentDetail* getSegmentDetail(const string& departureCity)
  {
    SegmentDetail* segmentDetail;
    segmentDetail = _memHandle.insert(new SegmentDetail);
    segmentDetail->departureCity() = LocCode(departureCity);
    return segmentDetail;
  }

  // tests
public:
  void testAddComponentPrefixInfo_SegmentDetailNotIn_MarketPosInFCLine()
  {
    _formatter->addComponentPrefixInfo(
        *_segmentDetail, string(""), *_pricingDetailTrx, *_paxDetail);
    CPPUNIT_ASSERT_EQUAL(string(""), _pricingDetailTrx->response().str());
  }

  void testAddComponentPrefixInfo_E()
  {
    string calcLine = "NYC LH E/FRA*CA X/L/BJS CA TYO1656.47YPXO NH OSA219.52YLNH /- FRA*LH LIS5M "
                      "NYCFRA6025.95C IB X/MAD AA X/T/NYC Q126.34AA SAO M1638.22Y JJ BUE Q6.00AR "
                      "LIM B/RIO15M1723.85JYGXFBR AA X/MIA Q115.00AA X/WAS AA NYC M2784.00Y P "
                      "NYCFRA NYCLIS175.05 D LIS MAD80.02COWEUIB NUC14550.42END ROE1.00 "
                      "XFJFK4.5JFK4.5MIA4.5 DCA4.5";
    _segmentDetail->departureCity() = LocCode("NYC");
    _formatter->_marketPosInFCLine[_segmentDetail] = pair<size_t, size_t>(0, 9);
    _formatter->addComponentPrefixInfo(*_segmentDetail, calcLine, *_pricingDetailTrx, *_paxDetail);
    CPPUNIT_ASSERT_EQUAL(string("                            E/FRA   TPM DEDUCTED-TRVL VIA NYC\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddComponentPrefixInfo_T()
  {
    string calcLine = "NYC LH E/FRA*CA X/L/BJS CA TYO1656.47YPXO NH OSA219.52YLNH /- FRA*LH LIS5M "
                      "NYCFRA6025.95C IB X/MAD AA X/T/NYC Q126.34AA SAO M1638.22Y JJ BUE Q6.00AR "
                      "LIM B/RIO15M1723.85JYGXFBR AA X/MIA Q115.00AA X/WAS AA NYC M2784.00Y P "
                      "NYCFRA NYCLIS175.05 D LIS MAD80.02COWEUIB NUC14550.42END ROE1.00 "
                      "XFJFK4.5JFK4.5MIA4.5 DCA4.5";
    _segmentDetail->departureCity() = LocCode("MAD");
    _formatter->_marketPosInFCLine[_segmentDetail] = pair<size_t, size_t>(95, 106);
    _formatter->addComponentPrefixInfo(*_segmentDetail, calcLine, *_pricingDetailTrx, *_paxDetail);
    CPPUNIT_ASSERT_EQUAL(string("                            T/NYC   MAD EXCLUDD FROM MILE CALC\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddComponentPrefixInfo_Stopover()
  {
    setSegmentDetail('T', 'N');
    _pricingDetailTrx->ticketingDate() = DateTime(2008, 9, 9);
    _paxDetail->constructionCurrencyCode() = "USD";
    _formatter->addComponentPrefixInfo(
        *_segmentDetail, string(""), *_pricingDetailTrx, *_paxDetail);
    CPPUNIT_ASSERT_EQUAL(string("               USD    0.002 NYC     STOPOVER SURCHARGE\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddComponentPrefixInfo_Transfer()
  {
    setSegmentDetail('N', 'T');
    _pricingDetailTrx->ticketingDate() = DateTime(2008, 9, 9);
    _paxDetail->constructionCurrencyCode() = "USD";
    _formatter->addComponentPrefixInfo(
        *_segmentDetail, string(""), *_pricingDetailTrx, *_paxDetail);
    CPPUNIT_ASSERT_EQUAL(string("               USD    0.002 NYC     TRANSFER SURCHARGE\n"),
                         _pricingDetailTrx->response().str());
  }

  void testCreateFCMarketMap_FareCalcDetailsEmpty()
  {
    _formatter->createFCMarketMap(*_paxDetail);
    CPPUNIT_ASSERT(_formatter->_marketPosInFCLine.empty());
  }

  void testCreateFCMarketMap_SegmentDetailsEmpty()
  {
    _paxDetail->fareCalcDetails().push_back(_memHandle.insert(new FareCalcDetail));
    _paxDetail->fareCalcDetails().push_back(_memHandle.insert(new FareCalcDetail));
    _formatter->createFCMarketMap(*_paxDetail);
    CPPUNIT_ASSERT(_formatter->_marketPosInFCLine.empty());
  }

  void testCreateFCMarketMap()
  {
    FareCalcDetail fcDetail;
    fcDetail.segmentDetails().push_back(getSegmentDetail("SAO"));
    fcDetail.segmentDetails().push_back(getSegmentDetail("NYC"));
    fcDetail.segmentDetails().push_back(getSegmentDetail("LON"));
    fcDetail.segmentDetails().push_back(getSegmentDetail("NYC"));

    _paxDetail->fareCalcDetails().push_back(&fcDetail);
    _paxDetail->fareCalcLine() = "14NOV09 SAO AA X/T/NYC AA LON Q111.00M1553.50YR AA X/T/NYC "
                                 "Q111.00AA SAO M1553.50YR NUC3329.00END ROE1.00 XFJFK4.5";

    _formatter->createFCMarketMap(*_paxDetail);

    CPPUNIT_ASSERT_EQUAL((size_t)4, _formatter->_marketPosInFCLine.size());
  }

  void testCreateFCMarketMap_2FareCalcDetails()
  {
    FareCalcDetail fcDetail1, fcDetail2;

    fcDetail1.segmentDetails().push_back(getSegmentDetail("SAO"));
    fcDetail1.segmentDetails().push_back(getSegmentDetail("NYC"));
    fcDetail2.segmentDetails().push_back(getSegmentDetail("LON"));
    fcDetail2.segmentDetails().push_back(getSegmentDetail("NYC"));

    _paxDetail->fareCalcDetails().push_back(&fcDetail1);
    _paxDetail->fareCalcDetails().push_back(&fcDetail2);
    _paxDetail->fareCalcLine() = "14NOV09 SAO AA X/T/NYC AA LON Q111.00M1553.50YR AA X/T/NYC "
                                 "Q111.00AA SAO M1553.50YR NUC3329.00END ROE1.00 XFJFK4.5";

    _formatter->createFCMarketMap(*_paxDetail);

    CPPUNIT_ASSERT_EQUAL((size_t)4, _formatter->_marketPosInFCLine.size());
  }

  void testAddFareBreakPointInfo_Without_SideTrip()
  {
    setFareCalcDetail();
    _formatter->addFareBreakPointInfo(
        *_fareCalcDetail, *_pricingDetailTrx, *_paxDetail, *_fcConfig, false);
    CPPUNIT_ASSERT_EQUAL(string(" REUNBA      GBP     0.25 CPH-LON BA /OW RTG              EH\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddFareBreakPointInfo_With_SideTrip()
  {
    setFareCalcDetail();
    _formatter->addFareBreakPointInfo(
        *_fareCalcDetail, *_pricingDetailTrx, *_paxDetail, *_fcConfig, true);
    CPPUNIT_ASSERT_EQUAL(string(" REUNBA     *GBP     0.25 CPH-LON BA /OW RTG              EH\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddFareBreakPointInfo_RW_SFC()
  {
    setFareCalcDetail();
    _fareCalcDetail->pricingUnitType() = "W";
    _formatter->addFareBreakPointInfo(
        *_fareCalcDetail, *_pricingDetailTrx, *_paxDetail, *_fcConfig, false);
    CPPUNIT_ASSERT_EQUAL(string(" REUNBA      GBP     0.25 CPH-LON BA /RT RTG              EH\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddFareBreakPointInfo_CT_SFC()
  {
    setFareCalcDetail();
    _fareCalcDetail->pricingUnitType() = "T";
    _formatter->addFareBreakPointInfo(
        *_fareCalcDetail, *_pricingDetailTrx, *_paxDetail, *_fcConfig, false);
    CPPUNIT_ASSERT_EQUAL(string(" REUNBA      GBP     0.25 CPH-LON BA /RT RTG              EH\n"),
                         _pricingDetailTrx->response().str());
  }

  void testAddPUType_RW_SFC()
  {
    setFareCalcDetail();
    _fareCalcDetail->pricingUnitType() = "W";
    _fareCalcDetail->pricingUnitCount() = 1;
    _paxDetail->fareCalcDetails().push_back(_fareCalcDetail);

    _formatter->addPUTripType(*_pricingDetailTrx, *_paxDetail);

    string expected = "RW SINGLE FARE COMPONENT";

    CPPUNIT_ASSERT(_pricingDetailTrx->response().str().find(expected) != string::npos);
  }

  void testAddPUType_CT_SFC()
  {
    setFareCalcDetail();
    _fareCalcDetail->pricingUnitType() = "T";
    _fareCalcDetail->pricingUnitCount() = 1;
    _paxDetail->fareCalcDetails().push_back(_fareCalcDetail);

    _formatter->addPUTripType(*_pricingDetailTrx, *_paxDetail);

    string expected = "CT SINGLE FARE COMPONENT";

    CPPUNIT_ASSERT(_pricingDetailTrx->response().str().find(expected) != string::npos);
  }

}; // class
CPPUNIT_TEST_SUITE_REGISTRATION(PricingDetailResponseFormatterTest);
} // namespace tse
