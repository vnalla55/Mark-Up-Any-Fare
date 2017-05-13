#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Tours.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/NetRemitPricing.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/test/PricingUtilTest.h"
#include "Rules/RuleConst.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"

#include <string>

namespace tse
{
FALLBACKVALUE_DECL(azPlusUp)

void
PricingUtilTest::setUp()
{
  _memHandle.create<TestConfigInitializer>();
  _memHandle.create<MyDataHandle>();
  _trx = _memHandle.create<PricingTrx>();
  _request = _memHandle.create<PricingRequest>();
  _trx->setRequest(_request);
  _discountPercentages = _memHandle.create<std::map<int16_t, Percent>>();
  _discountAmounts = _memHandle.create<std::vector<DiscountAmount>>();

  _options = _memHandle.create<PricingOptions>();
  _trx->setOptions(_options);

  _agent = new Agent();
  _request->ticketingAgent() = _agent;

  Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  _request->ticketingAgent()->agentLocation() = loc;

  _farePath = _memHandle.create<FarePath>();
  _farePath->setTotalNUCAmount(1000.00);

  _pricingUnit = _memHandle.create<PricingUnit>();
  _farePath->pricingUnit().push_back(_pricingUnit);

  _fareUsage = _memHandle.create<FareUsage>();
  _pricingUnit->fareUsage().push_back(_fareUsage);

  _travelSeg = _memHandle.create<AirSeg>();
  _travelSeg->segmentOrder() = 1;
  _fareUsage->travelSeg().push_back(_travelSeg);

  _fareMarket = _memHandle.create<FareMarket>();

  _ptf = _memHandle.create<PaxTypeFare>();
  _ptf->fareMarket() = _fareMarket;
  _fareUsage->paxTypeFare() = _ptf;

  _fare = _memHandle.create<Fare>();
  _fare->nucFareAmount() = 1000.0;
  _fare->nucOriginalFareAmount() = 1000.0;
  _ptf->setFare(_fare);

  _fareInfo = _memHandle.create<FareInfo>();
  _fareInfo->owrt() = ONE_WAY_MAY_BE_DOUBLED;
  _fare->setFareInfo(_fareInfo);
  _ptf->setFare(_fare);

  _farePath->calculationCurrency() = NUC;
  _farePath->baseFareCurrency() = "USD";
}

void
PricingUtilTest::tearDown()
{
  _memHandle.clear();
}

void
PricingUtilTest::addTourCode(PaxTypeFare& fare, const std::string& tourCode)
{
  Tours* tours = _memHandle.create<Tours>();
  tours->tourNo() = tourCode;

  PaxTypeFareRuleData* ruleData = _memHandle.create<PaxTypeFareRuleData>();
  ruleData->ruleItemInfo() = tours;

  PaxTypeFare::PaxTypeFareAllRuleData* data =
      _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
  data->fareRuleData = ruleData;

  (*fare.paxTypeFareRuleDataMap())[RuleConst::TOURS_RULE] = data;
}

void
PricingUtilTest::testDetermineCat27TourCodeEmptyFarePath()
{
  FarePath farePath;

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT(farePath.cat27TourCode().empty());
}

void
PricingUtilTest::testDetermineCat27TourCodeOneEmptyPricingUnit()
{
  FarePath farePath;
  PricingUnit emptyPricingUnit;
  farePath.pricingUnit().push_back(&emptyPricingUnit);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT(farePath.cat27TourCode().empty());
}

void
PricingUtilTest::testDetermineCat27TourCodeOnePricingUnitEmptyFareUsage()
{
  FarePath farePath;
  PricingUnit pricingUnit;
  FareUsage emptyFareUsage;

  farePath.pricingUnit().push_back(&pricingUnit);
  pricingUnit.fareUsage().push_back(&emptyFareUsage);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT(farePath.cat27TourCode().empty());
}

void
PricingUtilTest::addPricingUnit(FarePath& farePath, const std::string& tourCode)
{
  PricingUnit* pricingUnit = _memHandle.create<PricingUnit>();
  farePath.pricingUnit().push_back(pricingUnit);

  FareUsage* fareUsage = _memHandle.create<FareUsage>();
  pricingUnit->fareUsage().push_back(fareUsage);

  PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
  addTourCode(*paxTypeFare, tourCode);
  fareUsage->paxTypeFare() = paxTypeFare;
}

void
PricingUtilTest::testDetermineCat27TourCodeOnePricingUnitNoTourCode()
{
  FarePath farePath;
  addPricingUnit(farePath, "");

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT(farePath.cat27TourCode().empty());
}

void
PricingUtilTest::testDetermineCat27TourCodeOnePricingUnitWithTourCode()
{
  const std::string TOURCODE = "TEST";

  FarePath farePath;
  addPricingUnit(farePath, TOURCODE);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(TOURCODE, farePath.cat27TourCode());
}

void
PricingUtilTest::testDetermineCat27TourCodeTwoPricingUnitsNoTours()
{
  FarePath farePath;
  addPricingUnit(farePath, "");
  addPricingUnit(farePath, "");

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT(farePath.cat27TourCode().empty());
}

void
PricingUtilTest::testDetermineCat27TourCodeTwoPricingUnitsOneTour1()
{
  const std::string TOURCODE = "TEST";

  FarePath farePath;
  addPricingUnit(farePath, TOURCODE);
  addPricingUnit(farePath, "");

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(TOURCODE, farePath.cat27TourCode());
}

void
PricingUtilTest::testDetermineCat27TourCodeTwoPricingUnitsOneTour2()
{
  const std::string TOURCODE = "TEST";

  FarePath farePath;
  addPricingUnit(farePath, "");
  addPricingUnit(farePath, TOURCODE);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(TOURCODE, farePath.cat27TourCode());
}

void
PricingUtilTest::testDetermineCat27TourCodeTwoPricingUnitsTwoTours1()
{
  const std::string TOURCODE1 = "TEST";
  const std::string TOURCODE2 = "ABCDEF";

  FarePath farePath;
  addPricingUnit(farePath, TOURCODE1);
  addPricingUnit(farePath, TOURCODE2);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(TOURCODE1, farePath.cat27TourCode());
}

void
PricingUtilTest::testDetermineCat27TourCodeTwoPricingUnitsTwoTours2()
{
  const std::string TOURCODE1 = "TEST";
  const std::string TOURCODE2 = "ABCDEF";

  FarePath farePath;
  addPricingUnit(farePath, TOURCODE2);
  addPricingUnit(farePath, TOURCODE1);

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(TOURCODE2, farePath.cat27TourCode());
}

void
PricingUtilTest::testDetermineCat27TourCodeMultiplePricingUnits()
{
  FarePath farePath;

  addPricingUnit(farePath, "TOURCODE1");
  addPricingUnit(farePath, "TOURCODE2");
  addPricingUnit(farePath, "TOURCODE3");
  addPricingUnit(farePath, "TOURCODE4");
  addPricingUnit(farePath, "");

  PricingUtil::determineCat27TourCode(farePath);
  CPPUNIT_ASSERT_EQUAL(std::string("TOURCODE1"), farePath.cat27TourCode());
}

void
PricingUtilTest::setUpForJLExempt()
{
  _trx = _memHandle.create<PricingTrx>();
  _request = _memHandle.create<PricingRequest>();
  _agent = _memHandle.create<Agent>();
  _customer = _memHandle.create<Customer>();
  _agent->agentTJR() = _customer;
  _request->ticketingAgent() = _agent;
  _trx->setRequest(_request);
  _options = _memHandle.create<PricingOptions>();
  _trx->setOptions(_options);

  _farePath = _memHandle.create<FarePath>();
  _itin = _memHandle.create<Itin>();
  _farePath->itin() = _itin;
  _airSeg1 = _memHandle.create<AirSeg>();
  _airSeg2 = _memHandle.create<AirSeg>();
  _airSeg1->segmentOrder() = 0;
  _airSeg2->segmentOrder() = 1;
  _itin->travelSeg().push_back(_airSeg1);
  _itin->travelSeg().push_back(_airSeg2);
  _fareUsage = _memHandle.create<FareUsage>();
  _fareUsage->travelSeg().push_back(_airSeg1);
  _fareUsage->travelSeg().push_back(_airSeg2);
  _pricingUnit = _memHandle.create<PricingUnit>();
  _pricingUnit->fareUsage().push_back(_fareUsage);
  _pricingUnit->travelSeg().push_back(_airSeg1);
  _pricingUnit->travelSeg().push_back(_airSeg1);
  _farePath->pricingUnit().push_back(_pricingUnit);
}

void
PricingUtilTest::testIsJLExemptAccntCode1()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;

  CPPUNIT_ASSERT(!PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptAccntCode2()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;
  _trx->getRequest()->accountCode() = "ACCOUNTCODE123";

  CPPUNIT_ASSERT(!PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptAccntCode3()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;
  _trx->getRequest()->accountCode() = "RX78MS06TM";

  CPPUNIT_ASSERT(PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptAccntCode4()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = true;

  CPPUNIT_ASSERT(!PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptAccntCode5()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = true;
  _trx->getRequest()->accCodeVec().push_back("ACCOUNTCODE123");
  _trx->getRequest()->accCodeVec().push_back("ACCOUNTCODE1234");

  CPPUNIT_ASSERT(!PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptAccntCode6()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = true;
  _trx->getRequest()->accCodeVec().push_back("RX78MS06TM");

  CPPUNIT_ASSERT(PricingUtil::isJLExemptAccntCode(*_trx));
}

void
PricingUtilTest::testIsJLExemptTktDesig1()
{
  setUpForJLExempt();
  CPPUNIT_ASSERT(!PricingUtil::isJLExemptTktDesig(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExemptTktDesig2()
{
  setUpForJLExempt();
  const TktDesignator memphis = "MEMPHIS";
  _trx->getRequest()->tktDesignator().insert(std::make_pair(0, memphis));

  CPPUNIT_ASSERT(!PricingUtil::isJLExemptTktDesig(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExemptTktDesig3()
{
  setUpForJLExempt();
  const TktDesignator jmbjl = "JMBJL";
  _trx->getRequest()->tktDesignator().insert(std::make_pair(0, jmbjl));

  CPPUNIT_ASSERT(PricingUtil::isJLExemptTktDesig(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExempt1()
{
  setUpForJLExempt();
  CPPUNIT_ASSERT(!PricingUtil::isJLExempt(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExempt2()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;
  _trx->getRequest()->accountCode() = "RX78MS06TM";

  const TktDesignator jmbjl = "JMBJL";
  _trx->getRequest()->tktDesignator().insert(std::make_pair(0, jmbjl));

  _trx->getOptions()->forceCorpFares() = true;
  _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";

  CPPUNIT_ASSERT(!PricingUtil::isJLExempt(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExempt3()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;
  _trx->getRequest()->accountCode() = "RX78MS06TM";

  const TktDesignator jmbjl = "JMBJL";
  _trx->getRequest()->tktDesignator().insert(std::make_pair(0, jmbjl));

  _trx->getOptions()->forceCorpFares() = false;
  _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";
  _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";

  CPPUNIT_ASSERT(!PricingUtil::isJLExempt(*_trx, *_farePath));
}

void
PricingUtilTest::testIsJLExempt4()
{
  setUpForJLExempt();
  _trx->getRequest()->isMultiAccCorpId() = false;
  _trx->getRequest()->accountCode() = "RX78MS06TM";

  const TktDesignator jmbjl = "JMBJL";
  _trx->getRequest()->tktDesignator().insert(std::make_pair(0, jmbjl));

  _trx->getOptions()->forceCorpFares() = true;
  _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "AXES";
  _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1J";

  CPPUNIT_ASSERT(PricingUtil::isJLExempt(*_trx, *_farePath));
}

void
PricingUtilTest::setUpForPrintTaxItem(const DiagnosticTypes diagType)
{
  _taxItem = _memHandle.create<TaxItem>();

  // TaxCodeReg* taxCodeReg = _memHandle.create<TaxCodeReg>();
  // taxCodeReg->taxAmt() = 4;
  //_taxItem->taxCodeReg() = taxCodeReg;
  _taxItem->taxAmt() = 4;

  Diagnostic* rootDiag = _memHandle.create<Diagnostic>(diagType);
  rootDiag->activate();

  _diag = _memHandle.create<DiagCollector>();
  _diag->diagnosticType() = diagType;
  _diag->rootDiag() = rootDiag;
}

void
PricingUtilTest::testPrintTaxItem605Enabled()
{
  setUpForPrintTaxItem(Diagnostic605);

  _diag->enable(Diagnostic605);
  PricingUtil::printTaxItem(*_taxItem, *_diag);

  CPPUNIT_ASSERT_MESSAGE("DiagCollector should not be empty.", "" != _diag->str());
}

void
PricingUtilTest::testPrintTaxItem605Disabled()
{
  setUpForPrintTaxItem(Diagnostic605);

  PricingUtil::printTaxItem(*_taxItem, *_diag);

  CPPUNIT_ASSERT_EQUAL(std::string(""), _diag->str());
}

void PricingUtilTest::testIntersectCarrierList()
{
  {
    // Test with 2 empty vectors
    std::vector<CarrierCode> list1;
    std::vector<CarrierCode> list2;
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be empty.", list1.size() == 0);
  }

  CarrierCode set1 [] = {"AA", "AB", "BA"};
  CarrierCode set2 [] = {"AB", "AA", "AF"};
  CarrierCode set3 [] = {"TM", "AA", "AF", "DL"};
  CarrierCode set4 [] = {"AB", "BA"};

  {
    // 3 item vectors, 2 items common
    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set2, set2 + 3);
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be size 2.", list1.size() == 2);
  }

  {
    // 3 item vector and 4 item Vector, 1 item common
    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set3, set3 + 4);
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be size 1.", list1.size() == 1);
  }

  {
    // 3 item vector and 0 item Vector, Should return empty vector
    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set1, set1 + 3);
    std::vector<CarrierCode> list2;
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be empty.", list1.size() == 0);
  }

  {
    // 0 item vector and 4 item Vector, Should return empty vector
    std::vector<CarrierCode> list1;
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set3, set3 + 4);
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be empty.", list1.size() == 0);
  }

  {
    // 4 item vector and 2 item Vector, No Common item
    std::vector<CarrierCode> list1;
    list1.insert(list1.begin(), set3, set3 + 4);
    std::vector<CarrierCode> list2;
    list2.insert(list2.begin(), set4, set4 + 2);
    PricingUtil::intersectCarrierList(list1, list2);
    CPPUNIT_ASSERT_MESSAGE("Result vector should be empty.", list1.size() == 0);
  }
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount()
{
  _discountPercentages->insert( {1, 50.0} );
  _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_discount2()
{
  _discountAmounts->push_back(DiscountAmount(200, "USD", 1, 1));
  _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(800.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(200.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(800.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp()
{
  _discountPercentages->insert( {1, -50.0} );
  _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1500.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-500.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1500.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOff_plusUp2()
{
  _discountAmounts->push_back(DiscountAmount(-200, "USD", 1, 1));
  _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1200.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-200.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1200.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount()
{
  fallback::value::azPlusUp.set(true);
  _discountPercentages->insert( {1, 50.0} );
  _trx->getRequest()->setDiscountPercentages(*_discountPercentages);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(500.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_discount2()
{
  fallback::value::azPlusUp.set(true);

  _discountAmounts->push_back(DiscountAmount(200, "USD", 1, 1));
  _trx->getRequest()->setDiscountAmounts(*_discountAmounts);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(800.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(200.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(800.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp()
{
  fallback::value::azPlusUp.set(true);
  _discountPercentages->insert( {1, -50.0} );
  _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testDiscountOrPlusUpPricing_fallbackAzPlusUpOn_plusUp2()
{
  fallback::value::azPlusUp.set(true);

  _discountAmounts->push_back(DiscountAmount(-200, "USD", 1, 1));
  _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
  PricingUtil::discountOrPlusUpPricing(*_trx, *_farePath);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.00, _farePath->getTotalNUCAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage->getDiscAmount(), 0.01);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.00, _fareUsage->totalFareAmount(), 0.01);
}

void PricingUtilTest::testGetManualAdjustmentAmountsPerFUHelper()
{
  std::vector<MoneyAmount> perFuAmount, perFuAdjustmentAmount;

  bool rc = false;

  // Test empty fu vector
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, 100, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);

  perFuAmount.push_back(50);
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, 100, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 1);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 100);

  perFuAmount.push_back(75);
  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, 100, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 2);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 100);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);

  perFuAmount.push_back(125);
  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, 100, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 100);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -30, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -30);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -50, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -50);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -51, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -51);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -75, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -75);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -78, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -78);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -130, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -50);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -75);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -5);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -250, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -50);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -75);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -125);

  // Too big negative adjustment
  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -251, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == false);

  // 0 adjustment
  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, 0, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 3);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);

  // Test different sized FU
  perFuAmount.clear();
  perFuAmount.assign(5, 10);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -5, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -5, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -15, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -25, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == 0);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -35, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == -5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == 0);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -45, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == -5);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -50, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == true);
  CPPUNIT_ASSERT(perFuAdjustmentAmount.size() == 5);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[0] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[1] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[2] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[3] == -10);
  CPPUNIT_ASSERT(perFuAdjustmentAmount[4] == -10);

  perFuAdjustmentAmount.clear();
  rc = PricingUtil::getManualAdjustmentAmountsPerFUHelper(perFuAmount, -55, perFuAdjustmentAmount);
  CPPUNIT_ASSERT(rc == false);
}

void PricingUtilTest::testGetAslMileageDiff()
{
  PaxTypeFare ptf;
  FarePath farePath;
  farePath.aslMslDiffAmount() = 0;
  ptf.mileageSurchargeAmt() = 0;
  ptf.nucFareAmount() = 100;

  MoneyAmount aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);

  ptf.mileageSurchargePctg() = 0;
  aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);

  ptf.mileageSurchargeAmt() = 10;
  aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);

  ptf.mileageSurchargeAmt() = 0;
  ptf.mileageSurchargePctg() = 10;

  aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);

  ptf.mileageSurchargePctg() = 15;
  ptf.mileageSurchargeAmt() = 10;

  aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 5);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 15);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 0);

  ptf.mileageSurchargeAmt() = 20;
  aslMileageDiff = PricingUtil::getAslMileageDiff(false, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == -5);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 15);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 0);

  // MSL active
  ptf.mileageSurchargeAmt() = 10;
  aslMileageDiff = PricingUtil::getAslMileageDiff(true, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 10);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 5);

  aslMileageDiff = PricingUtil::getAslMileageDiff(true, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 10);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 10);

  ptf.mileageSurchargeAmt() = 20;
  aslMileageDiff = PricingUtil::getAslMileageDiff(true, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 20);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 5);

  aslMileageDiff = PricingUtil::getAslMileageDiff(true, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 20);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == 0);

  aslMileageDiff = PricingUtil::getAslMileageDiff(true, ptf, farePath);
  CPPUNIT_ASSERT(aslMileageDiff == 0);
  CPPUNIT_ASSERT(ptf.mileageSurchargeAmt() == 20);
  CPPUNIT_ASSERT(farePath.aslMslDiffAmount() == -5);
}

void PricingUtilTest::testConvertCurrencyForMsl()
{
  _trx->getRequest()->ticketingDT() = DateTime::localTime();

  // exchange rate is 1.23456. No rounding should happen
  MoneyAmount amount = PricingUtil::convertCurrencyForMsl(*_trx, 100, "GBP", "USD");
  std::cout << "ooo " << amount << " ooo\n";
  CPPUNIT_ASSERT(fabs(amount - 123.456) < EPSILON);

  amount = PricingUtil::convertCurrencyForMsl(*_trx, -100, "GBP", "USD");
  CPPUNIT_ASSERT(fabs(amount + 123.456) < EPSILON);

  amount = PricingUtil::convertCurrencyForMsl(*_trx, 100, "USD", "USD");
  CPPUNIT_ASSERT(amount == 100);

  amount = PricingUtil::convertCurrencyForMsl(*_trx, -100, "USD", "USD");
  CPPUNIT_ASSERT(amount == -100);
}

void PricingUtilTest::testAdjustedSellingCalcDataExists()
{
  PaxTypeFare ptfAdj, ptf1, ptf2;
  AdjustedSellingCalcData adjSellCalcData;
  ptfAdj.setAdjustedSellingCalcData(&adjSellCalcData);

  FarePath farePath;
  CPPUNIT_ASSERT(!PricingUtil::adjustedSellingCalcDataExists(farePath));

  PricingUnit puAdj, pu1, pu2;
  FareUsage fuAdj, fu1, fu2;
  fuAdj.paxTypeFare() = &ptfAdj;
  fu1.paxTypeFare() = &ptf1;
  fu2.paxTypeFare() = &ptf2;

  puAdj.fareUsage().push_back(&fuAdj);
  pu1.fareUsage().push_back(&fu1);
  pu2.fareUsage().push_back(&fu2);

  farePath.pricingUnit().push_back(&pu1);
  CPPUNIT_ASSERT(!PricingUtil::adjustedSellingCalcDataExists(farePath));

  farePath.pricingUnit().push_back(&pu2);
  CPPUNIT_ASSERT(!PricingUtil::adjustedSellingCalcDataExists(farePath));

  farePath.pricingUnit().push_back(&puAdj);
  CPPUNIT_ASSERT(PricingUtil::adjustedSellingCalcDataExists(farePath));
}

void PricingUtilTest::testProcessAdjustedSellingFarePathASLPositive()
{
  FarePath farePath;
  Itin itin;
  farePath.itin() = &itin;

  farePath.calculationCurrency() = NUC;

  PaxTypeFare ptfAdj, ptf1;

  AdjustedSellingCalcData adjSellCalcData;
  adjSellCalcData.setCalculatedAmt(120);
  adjSellCalcData.setCalculatedNucAmt(120);
  ptfAdj.setAdjustedSellingCalcData(&adjSellCalcData);

  PricingUnit puAdj, pu1;
  FareUsage fuAdj, fu1, fu2;
  fuAdj.paxTypeFare() = &ptfAdj;
  fu1.paxTypeFare() = &ptf1;

  puAdj.fareUsage().push_back(&fuAdj);
  pu1.fareUsage().push_back(&fu1);

  Fare fare;
  FareInfo fareInfo;
  fare.setFareInfo(&fareInfo);
  ptf1.setFare(&fare);

  farePath.pricingUnit().push_back(&pu1);

  FareMarket fm;
  ptf1.fareMarket() = &fm;

  Fare fare2;
  FareInfo fareInfo2;
  fare2.setFareInfo(&fareInfo2);
  ptfAdj.setFare(&fare2);
  ptfAdj.nucFareAmount() = 100;

  farePath.pricingUnit().push_back(&puAdj);

  ptf1.fareMarket() = &fm;
  ptfAdj.fareMarket() = &fm;

  puAdj.setTotalPuNucAmount(600);
  pu1.setTotalPuNucAmount(400);
  farePath.setTotalNUCAmount(1000);

  PricingUtil::processAdjustedSellingFarePath(*_trx, farePath, false);

  FarePath* adjustedFP = farePath.adjustedSellingFarePath();
  CPPUNIT_ASSERT(adjustedFP);
  CPPUNIT_ASSERT(adjustedFP->isAdjustedSellingFarePath());

  // No changes are expected for original fare path
  CPPUNIT_ASSERT(farePath.getTotalNUCAmount() == 1000);
  CPPUNIT_ASSERT(pu1.getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(puAdj.getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(adjustedFP->getTotalNUCAmount() == 1020);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit().size() == 2);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->getTotalPuNucAmount() == 620);

  CPPUNIT_ASSERT(farePath.pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 120);
}

void PricingUtilTest::testProcessAdjustedSellingFarePathASLNegative()
{
  FarePath farePath;
  Itin itin;
  farePath.itin() = &itin;

  farePath.calculationCurrency() = NUC;

  PaxTypeFare ptfAdj, ptf1;

  AdjustedSellingCalcData adjSellCalcData;
  adjSellCalcData.setCalculatedAmt(80);
  adjSellCalcData.setCalculatedNucAmt(80);
  ptfAdj.setAdjustedSellingCalcData(&adjSellCalcData);

  PricingUnit puAdj, pu1;
  FareUsage fuAdj, fu1, fu2;
  fuAdj.paxTypeFare() = &ptfAdj;
  fu1.paxTypeFare() = &ptf1;

  puAdj.fareUsage().push_back(&fuAdj);
  pu1.fareUsage().push_back(&fu1);

  Fare fare;
  FareInfo fareInfo;
  fare.setFareInfo(&fareInfo);
  ptf1.setFare(&fare);

  farePath.pricingUnit().push_back(&pu1);

  FareMarket fm;
  ptf1.fareMarket() = &fm;

  Fare fare2;
  FareInfo fareInfo2;
  fare2.setFareInfo(&fareInfo2);
  ptfAdj.setFare(&fare2);
  ptfAdj.nucFareAmount() = 100;

  farePath.pricingUnit().push_back(&puAdj);

  ptf1.fareMarket() = &fm;
  ptfAdj.fareMarket() = &fm;

  puAdj.setTotalPuNucAmount(600);
  pu1.setTotalPuNucAmount(400);
  farePath.setTotalNUCAmount(1000);

  PricingUtil::processAdjustedSellingFarePath(*_trx, farePath, false);

  FarePath* adjustedFP = farePath.adjustedSellingFarePath();
  CPPUNIT_ASSERT(adjustedFP);
  CPPUNIT_ASSERT(adjustedFP->isAdjustedSellingFarePath());

  // No changes are expected for original fare path
  CPPUNIT_ASSERT(farePath.getTotalNUCAmount() == 1000);
  CPPUNIT_ASSERT(pu1.getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(puAdj.getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(adjustedFP->getTotalNUCAmount() == 980);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit().size() == 2);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->getTotalPuNucAmount() == 580);

  CPPUNIT_ASSERT(farePath.pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 80);
}

void PricingUtilTest::testProcessAdjustedSellingFarePathMSLPositive()
{
  FarePath farePath;
  Itin itin;
  farePath.itin() = &itin;

  farePath.calculationCurrency() = NUC;
  _trx->getOptions()->currencyOverride() = NUC;

  PaxType paxType;
  paxType.mslAmount() = 77;

  farePath.paxType() = &paxType;

  PaxTypeFare ptfAdj, ptf1;

  PricingUnit puAdj, pu1;
  FareUsage fuAdj, fu1, fu2;
  fuAdj.paxTypeFare() = &ptfAdj;
  fu1.paxTypeFare() = &ptf1;

  puAdj.fareUsage().push_back(&fuAdj);
  pu1.fareUsage().push_back(&fu1);

  Fare fare;
  FareInfo fareInfo;
  fareInfo.currency() = NUC;
  fare.setFareInfo(&fareInfo);
  ptf1.setFare(&fare);
  ptf1.nucFareAmount() = 200;

  farePath.pricingUnit().push_back(&pu1);

  FareMarket fm;
  ptf1.fareMarket() = &fm;

  Fare fare2;
  FareInfo fareInfo2;
  fareInfo2.currency() = NUC;
  fare2.setFareInfo(&fareInfo2);
  ptfAdj.setFare(&fare2);
  ptfAdj.nucFareAmount() = 100;

  farePath.pricingUnit().push_back(&puAdj);

  ptf1.fareMarket() = &fm;
  ptfAdj.fareMarket() = &fm;

  puAdj.setTotalPuNucAmount(600);
  pu1.setTotalPuNucAmount(400);
  farePath.setTotalNUCAmount(1000);

  PricingUtil::processAdjustedSellingFarePath(*_trx, farePath, true);

  FarePath* adjustedFP = farePath.adjustedSellingFarePath();
  CPPUNIT_ASSERT(adjustedFP);
  CPPUNIT_ASSERT(adjustedFP->isAdjustedSellingFarePath());

  // No changes are expected for original fare path
  CPPUNIT_ASSERT(farePath.getTotalNUCAmount() == 1000);
  CPPUNIT_ASSERT(pu1.getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(puAdj.getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(adjustedFP->getTotalNUCAmount() == 1077);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit().size() == 2);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->getTotalPuNucAmount() == 477);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(farePath.pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 200);
  CPPUNIT_ASSERT(farePath.pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 277);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
}

void PricingUtilTest::testProcessAdjustedSellingFarePathMSLNegative()
{
  FarePath farePath;
  Itin itin;
  farePath.itin() = &itin;

  farePath.calculationCurrency() = NUC;
  _trx->getOptions()->currencyOverride() = NUC;

  PaxType paxType;
  paxType.mslAmount() = -55;

  farePath.paxType() = &paxType;

  PaxTypeFare ptfAdj, ptf1;

  PricingUnit puAdj, pu1;
  FareUsage fuAdj, fu1, fu2;
  fuAdj.paxTypeFare() = &ptfAdj;
  fu1.paxTypeFare() = &ptf1;

  puAdj.fareUsage().push_back(&fuAdj);
  pu1.fareUsage().push_back(&fu1);

  Fare fare;
  FareInfo fareInfo;
  fareInfo.currency() = NUC;
  fare.setFareInfo(&fareInfo);
  ptf1.setFare(&fare);
  ptf1.nucFareAmount() = 200;

  farePath.pricingUnit().push_back(&pu1);

  FareMarket fm;
  ptf1.fareMarket() = &fm;

  Fare fare2;
  FareInfo fareInfo2;
  fareInfo2.currency() = NUC;
  fare2.setFareInfo(&fareInfo2);
  ptfAdj.setFare(&fare2);
  ptfAdj.nucFareAmount() = 100;

  farePath.pricingUnit().push_back(&puAdj);

  ptf1.fareMarket() = &fm;
  ptfAdj.fareMarket() = &fm;

  puAdj.setTotalPuNucAmount(600);
  pu1.setTotalPuNucAmount(400);
  farePath.setTotalNUCAmount(1000);

  PricingUtil::processAdjustedSellingFarePath(*_trx, farePath, true);

  FarePath* adjustedFP = farePath.adjustedSellingFarePath();
  CPPUNIT_ASSERT(adjustedFP);
  CPPUNIT_ASSERT(adjustedFP->isAdjustedSellingFarePath());

  // No changes are expected for original fare path
  CPPUNIT_ASSERT(farePath.getTotalNUCAmount() == 1000);
  CPPUNIT_ASSERT(pu1.getTotalPuNucAmount() == 400);
  CPPUNIT_ASSERT(puAdj.getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(adjustedFP->getTotalNUCAmount() == 945);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit().size() == 2);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->getTotalPuNucAmount() == 345);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->getTotalPuNucAmount() == 600);

  CPPUNIT_ASSERT(farePath.pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 200);
  CPPUNIT_ASSERT(farePath.pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[0]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 145);
  CPPUNIT_ASSERT(adjustedFP->pricingUnit()[1]->fareUsage()[0]->paxTypeFare()->nucFareAmount() == 100);
}

} // tse
