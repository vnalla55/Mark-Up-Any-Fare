#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Rules/RuleConst.h"
#include "Common/Money.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "Fares/AdjustedFareCalc.h"
#include "Fares/AdjustedSellingLevelFareCollector.h"
#include "Fares/FareRetailerRuleValidator.h"
#include "test/include/MockGlobal.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/FareDisplayTrx.h"

using namespace std;

namespace tse
{
namespace
{
}

class AdjustedSellingLevelFareCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdjustedSellingLevelFareCollectorTest);

  CPPUNIT_TEST(test_checkAge);
  CPPUNIT_TEST(test_updateFareMarket);
  CPPUNIT_TEST(test_getSellingInfo);
  CPPUNIT_TEST(test_isSuitableCalcDetail);
  CPPUNIT_TEST(test_checkPtfValidity);
  CPPUNIT_TEST(test_keepFareRetailerMinAmt);

  CPPUNIT_TEST_SUITE_END();

public:

  void test_checkAge()
  {
    bool rc = _adjustedSellingLevelFareCollector->checkAge(*_paxTypeFare, *_paxType);
    CPPUNIT_ASSERT_EQUAL(rc, true);

    _paxType->age() = 8;
    rc = _adjustedSellingLevelFareCollector->checkAge(*_paxTypeFare, *_paxType);
    CPPUNIT_ASSERT_EQUAL(rc, false);

     _paxType->age() = 68;
     rc = _adjustedSellingLevelFareCollector->checkAge(*_paxTypeFare, *_paxType);
     CPPUNIT_ASSERT_EQUAL(rc, false);

     _paxType->age() = 0;
     rc = _adjustedSellingLevelFareCollector->checkAge(*_paxTypeFare, *_paxType);
     CPPUNIT_ASSERT_EQUAL(rc, true);
  }

  void test_updateFareMarket()
  {
    _adjustedSellingLevelFareCollector->updateFareMarket(_fm);
    size_t sz = _fm->allPaxTypeFare().size();
    CPPUNIT_ASSERT_EQUAL(sz, (size_t)2);

    _adjustedSellingLevelFareCollector->updateFareMarket(_fm);
    sz = _fm->allPaxTypeFare().size();
    CPPUNIT_ASSERT_EQUAL(sz, (size_t)3);
  }

  void test_getSellingInfo()
  {
    _adjustedSellingLevelFareCollector->getSellingInfo(_adjustedSellingCalcData,
                                                       *_adjustedFareCalc);

    CPPUNIT_ASSERT_EQUAL(_adjustedSellingCalcData->getPercent(), 10.0);
    CPPUNIT_ASSERT_EQUAL(_adjustedSellingCalcData->getRuleAmount(), 200.0);
    CPPUNIT_ASSERT_EQUAL(_adjustedSellingCalcData->getNoDecPercent(), 10);
    CPPUNIT_ASSERT_EQUAL(_adjustedSellingCalcData->getNoDecAmt(), 200);
  }

  void test_isSuitableCalcDetail()
  {
    _frCalcDetInfo->calculationTypeCd()="SL";
    bool rc = _adjustedSellingLevelFareCollector->isSuitableCalcDetail(_frCalcDetInfo);
    CPPUNIT_ASSERT_EQUAL(rc, true);

    _frCalcDetInfo->calculationTypeCd()="SS";
    rc = _adjustedSellingLevelFareCollector->isSuitableCalcDetail(_frCalcDetInfo);
    CPPUNIT_ASSERT_EQUAL(rc, false);
  }

  void test_checkPtfValidity()
  {
    PaxTypeFare *_ptff=0;
    bool rc = _adjustedSellingLevelFareCollector->checkPtfValidity(_ptff, true);
    CPPUNIT_ASSERT_EQUAL(rc, false);

    rc = _adjustedSellingLevelFareCollector->checkPtfValidity(_paxTypeFare, true);
    CPPUNIT_ASSERT_EQUAL(rc, true);

    _paxTypeFare->setMatchFareFocusRule(true);
    rc = _adjustedSellingLevelFareCollector->checkPtfValidity(_paxTypeFare, false);
    CPPUNIT_ASSERT_EQUAL(rc, false);

    _paxTypeFare->setMatchFareFocusRule(false);
    rc = _adjustedSellingLevelFareCollector->checkPtfValidity(_paxTypeFare, false);
    CPPUNIT_ASSERT_EQUAL(rc, true);
  }

  void test_keepFareRetailerMinAmt()
  {
    MoneyAmount minAmt = 50.0;
    MoneyAmount minAmtNuc = 55.0;
    MoneyAmount currAmt = 100.0;
    MoneyAmount currAmtNuc = 105.0;
    bool rc = _adjustedSellingLevelFareCollector->keepFareRetailerMinAmt
       (_paxTypeFare, minAmt, minAmtNuc, currAmt, currAmtNuc,
        *_adjustedFareCalc, _adjustedSellingCalcData, *_context, _frCalcDetInfo);
    CPPUNIT_ASSERT_EQUAL(rc, false);

    minAmt = minAmtNuc = 200.0;
    rc = _adjustedSellingLevelFareCollector->keepFareRetailerMinAmt
        (_paxTypeFare, minAmt, minAmtNuc, currAmt, currAmtNuc,
        *_adjustedFareCalc, _adjustedSellingCalcData, *_context, _frCalcDetInfo);
    CPPUNIT_ASSERT_EQUAL(rc, true);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    DateTime now = DateTime::localTime();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->ticketingDT() = now;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(req);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    _fm = _memHandle.create<FareMarket>();
    _fm->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    _fm->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _fm->boardMultiCity() = _fm->origin()->loc();
    _fm->offMultiCity() = _fm->destination()->loc();

    // FARE
    _paxType = _memHandle.create<PaxType>();
    _paxType->age() = 20;

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->setIsShoppingFare();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();

    _fareInfo->carrier() = "AA";
    _fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _fareInfo->originalFareAmount() = 100.0;
    _fareInfo->fareAmount() = 50.0;
    _fare->setFareInfo(_fareInfo);

    _paxTypeFare->setFare(_fare);
    _paxTypeFare->nucOriginalFareAmount() = 300.0;
    _paxTypeFare->nucFareAmount() = 150.0;

    _fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();
    _fareClassAppSegInfo->_minAge = 15;
    _fareClassAppSegInfo->_maxAge = 65;

    _paxTypeFare->fareClassAppSegInfo()  = _fareClassAppSegInfo;
    _paxTypeFare->fareDisplayCat35Type() = RuleConst::SELLING_CARRIER_FARE;

    Itin* itin = _memHandle.create<Itin>();
    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->departureDT() = now;
    itin->travelSeg().push_back(travelSeg);
    itin->calculationCurrency() = "USD";
    itin->useInternationalRounding() = true;

    _trx->travelSeg().push_back(travelSeg);
    _fm->travelSeg().push_back(travelSeg);
    _fm->fbcUsage() = COMMAND_PRICE_FBC;
    _fm->allPaxTypeFare().push_back(_paxTypeFare);
    _paxTypeFare->fareMarket() = _fm;

    _adjustedSellingCalcData = _memHandle.insert(new AdjustedSellingCalcData());
    _adjustedFareCalc = _memHandle.insert(new AdjustedFareCalc(*_trx, *itin));

    _frCalcDetInfo = _memHandle.create<FareRetailerCalcDetailInfo>();
    _frCalcDetInfo->fareCalcInd() = 'C';
    _frCalcDetInfo->percent1() = 10.0;
    _frCalcDetInfo->percent2() = 20.0;
    _frCalcDetInfo->amount1() = 200.0;
    _frCalcDetInfo->amountCurrency1() = "USD";
    _frCalcDetInfo->amount2() = 300.0;
    _frCalcDetInfo->amountCurrency2() = "EUR";
    _frCalcDetInfo->percentNoDec1() = 10;
    _frCalcDetInfo->percentNoDec2() = 20;
    _frCalcDetInfo->amountNoDec1() = 200;
    _frCalcDetInfo->amountNoDec2() = 300;

    _adjustedFareCalc->load(*_frCalcDetInfo);
    _adjustedSellingLevelFareCollector =  _memHandle.insert(new AdjustedSellingLevelFareCollector(*_trx));
    _adjustedSellingLevelFareCollector->fareDisplayPtfVec().push_back(_paxTypeFare);

    _frri = _memHandle.create<FareRetailerRuleInfo>();
    _frri->fareRetailerRuleId() = 10;
    _frri->ruleSeqNo() = 20;

    PseudoCityCode sourcePcc = "ABC";
    _context=_memHandle.insert(new FareRetailerRuleContext(sourcePcc,
                                                           _frri,
                                                           _frciV,
                                                           _frrfai,
                                                           _ffsi));
    _adjustedSellingLevelFareCollector->_fdTrx =
        dynamic_cast<FareDisplayTrx*>(&_adjustedSellingLevelFareCollector->_trx);
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;

  PricingTrx* _trx;
  PricingOptions* _options;
  Fare* _fare;
  FareMarket* _fm;
  FareInfo* _fareInfo;
  PaxTypeFare* _paxTypeFare;
  PaxType* _paxType;
  FareClassAppSegInfo* _fareClassAppSegInfo;
  AdjustedFareCalc* _adjustedFareCalc;
  AdjustedSellingCalcData* _adjustedSellingCalcData;
  AdjustedSellingLevelFareCollector* _adjustedSellingLevelFareCollector;
  FareRetailerCalcDetailInfo* _frCalcDetInfo;
  struct FareRetailerRuleContext* _context;
  std::vector<FareRetailerCalcInfo*> _frciV;
  FareRetailerResultingFareAttrInfo* _frrfai;
  FareFocusSecurityInfo* _ffsi;
  FareRetailerRuleInfo* _frri;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AdjustedSellingLevelFareCollectorTest);
}
